#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <regex>
#include "tupleObjects.h"
#include "tokenizer.h"
#include "lindaFunctions.h"

std::vector<lindaTuple> globalTuples;
pthread_mutex_t gLock;
std::vector<pthread_cond_t *> threadLocks;
std::vector<pthread_cond_t *> waitingList;

void out(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  lindaTuple newTuple;
  if (generateOutTuple(elems, newTuple, localVars, userDefinedFuncs, loopSymbols))
    // Use global tuples
    pthread_mutex_lock(&gLock);
    globalTuples.push_back(newTuple);
    pthread_mutex_unlock(&gLock);
    // Notify all waiting threads
    std::cout << "Wait list size found " << waitingList.size() << std::endl;
    for (size_t i = 0; i < waitingList.size(); i++) {
      pthread_cond_signal(waitingList[i]);
    }
}

void in(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum) {

  pthread_mutex_lock(&gLock);
  std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols);
  while (1) {
    if (result == globalTuples.end()) {
      std::cout << "block from in" << std::endl;
      pthread_cond_wait(threadLocks[threadNum], &gLock);
      waitingList.push_back(threadLocks[threadNum]);
      std::cout << "WaitingList length " << waitingList.size() << std::endl;
    } else {
      globalTuples.erase(result);
      // Remove from waiting list
      //waitingList.
      break;
    }
  }
  pthread_mutex_unlock(&gLock);
}

void rd(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum) {
  pthread_mutex_lock(&gLock);
  std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols);
  while (1) {
    if (result == globalTuples.end()) {
      //std::cout << "block from rd" << std::endl;
      pthread_cond_wait(threadLocks[threadNum], &gLock);
      waitingList.push_back(threadLocks[threadNum]);
    } else {
      break;
    }
  }
  pthread_mutex_unlock(&gLock);
}

bool inp(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols);
  if (result == globalTuples.end()) 
    return false;
  globalTuples.erase(result);
  return true;
}

bool rdp(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  return findInTuple(elems, localVars, userDefinedFuncs, loopSymbols) != globalTuples.end();
}

void eval(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  for (size_t i = 0; i < elems.size(); i++) {
    if (isExp(elems[i])) {
      // first case: get expression result
      int result = evaluateExp(elems[i], loopSymbols, userDefinedFuncs, localVars);
      if (result != -1)
	elems[i] = std::to_string(result);
      else {
	// block and wait
      }
    } else if (loopSymbols.find(elems[i]) != loopSymbols.end()) {
      //second case: Change loop symbol to value
      elems[i] = std::to_string(loopSymbols[elems[i]]);
    }
  }

  lindaTuple newTuple;
  if (generateOutTuple(elems, newTuple, localVars, userDefinedFuncs, loopSymbols))
    globalTuples.push_back(newTuple);
}

void dump() {
  for (std::vector<lindaTuple>::iterator it = globalTuples.begin(); it != globalTuples.end(); it++) {
    std::cout << *it << std::endl;
  }
}

void lindaFor(int start, int end, std::string symbol, std::vector<std::string>lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum) {
  //for (auto line:lines) std::cout << line << std::endl;
  for (int i = start; i < end; i++) {
    loopSymbols[symbol] = i;
    for (std::vector<std::string>::iterator it = lines.begin() + 1; it != lines.end(); it++) {
      LINDA_TYPE type = findFunctionType(*it);
      if (isOneLineCommand(type)) {
	std::vector<std::string> newLines(it, it + 1);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
      } else if (isMultiLineCommand(type)) {
	std::vector<std::string> newLines = getMultiLines(lines, it);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
      }
    }
  }
  loopSymbols.erase(symbol);
}

void lindaIF(std::vector<std::string>lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum) {
  size_t start = (lines[0]).find("(") + 1;
  size_t end = (lines[0]).find_last_of(")");
  std::string condition = (lines[0]).substr(start, end - start);

  std::vector<std::string> ifExps;
  std::vector<std::string> elseExps;
  bool ifcase = true;
  for (auto it = lines.begin() + 1; it != lines.end(); it++) {
    if ((*it).find("else") != std::string::npos)
      ifcase = false;
    if (ifcase)
      ifExps.push_back(*it);
    else
      elseExps.push_back(*it);
  }

  // Evaluate condition
  if (evaluateExp(condition, loopSymbols, userDefinedFuncs, localVars)) {
    //std::cout << "True" << std::endl;
    for (std::vector<std::string>::iterator it = ifExps.begin(); it != ifExps.end(); it++) {
      LINDA_TYPE type = findFunctionType(*it);
      if (isOneLineCommand(type)) {
	std::vector<std::string> newLines(it, it + 1);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
      } else if (isMultiLineCommand(type)) {
	std::vector<std::string> newLines = getMultiLines(lines, it);
	//for (auto line:newLines) std::cout << line << std::endl;
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
      }
    }
  } else {
    //std::cout << "false" << std::endl;
    for (std::vector<std::string>::iterator it = elseExps.begin(); it != elseExps.end(); it++) {
      LINDA_TYPE type = findFunctionType(*it);
      if (isOneLineCommand(type)) {
	std::vector<std::string> newLines(it, it + 1);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
      } else if (isMultiLineCommand(type)) {
	std::vector<std::string> newLines = getMultiLines(lines, it);
	//for (auto line:newLines) std::cout << line << std::endl;
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
      }
    }
  }
}


void runFunc(LINDA_TYPE type, std::vector<std::string> &lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum) {
  std::vector<std::string> elems;
  int start, end;
  std::string symbol;
  switch (type) {
  case IN:
    std::cout << "Thread" << threadNum << " in" << std::endl;
    getInOutElems(lines[0], elems);
    in(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case OUT:
    std::cout << "Thread" << threadNum << " out" << std::endl;
    getInOutElems(lines[0], elems);
    out(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case EVAL:
    std::cout << "Thread" << threadNum << " eval" << std::endl;
    getInOutElems(lines[0], elems);
    eval(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case RD:
    std::cout << "Thread" << threadNum << " rdp" << std::endl;
    getInOutElems(lines[0], elems);
    rd(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case RDP:
    std::cout << "Thread" << threadNum << " rdp" << std::endl;
    getInOutElems(lines[0], elems);
    rdp(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case INP:
    std::cout << "Thread" << threadNum << " inp" << std::endl;
    getInOutElems(lines[0], elems);
    inp(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case DUMP:
    std::cout << "Thread" << threadNum << " dump" << std::endl;
    dump();
    break;
  case IF:
    std::cout << "Thread" << threadNum << " if" << std::endl;
    //for (auto f:lines) std::cout << f << std::endl;
    lindaIF(lines, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case FOR:
    std::cout << "Thread" << threadNum << " for" << std::endl;
    symbol = getForParams(lines[0], &start, &end);
    //std::cout << "Params " << start << end << symbol << std::endl;
    lindaFor(start, end, symbol, lines, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case DEFINE:
    std::cout << "Thread" << threadNum << " define" << std::endl;
    writeFile(lines, userDefinedFuncs, threadNum);
    //for (auto f:userDefinedFuncs) std::cout << f << std::endl;
    break;
  default:
    std::cout << "Thread" << threadNum << " other" << std::endl;
  }
}

void *threadProcessor(void *args) {
  // Open file
  char *pos = strrchr((char *)args, '#');
  char *number = pos + 1;
  *pos = '\0';
  char *filename = (char *)args;
  int threadNum = atoi(number);
  std::ifstream infile(filename);
  if (!infile) {
    std::cout << "Can not open file " << filename << std::endl;
    exit(EXIT_FAILURE);
  }

  // Build all local variables' containers 
  std::map<std::string, lindaObj *> localVars;
  std::set<std::string> userDefinedFuncs;
  std::map<std::string, int> loopSymbols;

  // Read file
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(infile, line)) {
    lines.push_back(line);
  }
  infile.close();

  // Process each line
  for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); it++) {
    // Get command type
    LINDA_TYPE type = findFunctionType(*it);
    // Run command
    if (isOneLineCommand(type)) {
      std::vector<std::string> newLines(it, it + 1);
      runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
    } else if (isMultiLineCommand(type)) {
      std::vector<std::string> newLines = getMultiLines(lines, it);
      //for (auto line:newLines) std::cout << line << std::endl;
      runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols, threadNum);
    } else {
      if (!line.empty()) {
	std::cout << "Can not recognize command " << *it << std::endl;
	exit(EXIT_FAILURE);
      }
    }
  }
  return NULL;
}

int main() {
  std::string filename;
  // Standard output and input
  std::cout << "Enter input file path: ";
  std::cin >> filename;
  // Open .bat file
  std::ifstream infile(filename.c_str());
  if (!infile) {
    std::cout << "Can not open file " << filename << std::endl;
    exit(EXIT_FAILURE);
  }
  // Read file
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(infile, line)) {
    lines.push_back(line);
  }
  infile.close();

  // Initialize mutex locks
  pthread_mutex_init(&gLock, NULL);

  // Initialize conditions locks
  int numThread = lines.size();
  for (int i = 0; i < numThread; i++) {
    pthread_cond_t *condition = new pthread_cond_t();
    pthread_cond_init(condition, NULL);
    threadLocks.push_back(condition);
  }
  // Create threads
  pthread_t allThreads[numThread];
  int status;
  void *exit_status;

  for (int i = 0; i < numThread; i++) {
    lines[i] = lines[i] + "#" + std::to_string(i);
    //std::cout << "args " << args << std::endl;
    status = pthread_create(allThreads + i, NULL, threadProcessor, (void *)(lines[i].c_str()));
    std::cout << "Create thread for file \"" << lines[i] << "\'" << std::endl;
    if (status != 0) {
      std::cout << "Create thread for file \"" << lines[i] << "\" error" << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < numThread; i++) {
    pthread_join(allThreads[i], &exit_status);
  }

  pthread_mutex_destroy(&gLock);
  for (size_t i = 0; i < threadLocks.size(); i++)
  pthread_cond_destroy(threadLocks[i]);
  return 0;
}
