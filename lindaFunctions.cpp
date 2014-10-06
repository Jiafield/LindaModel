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
#include "lindaFunctions.h"
#include "tokenizer.h"

std::vector<lindaTuple> globalTuples;
pthread_mutex_t gLock;    //Global tuples' lock
std::vector<pthread_cond_t *> threadLocks;
std::vector<pthread_cond_t *> waitingList;
pthread_mutex_t wLock;    // Waiting list lock
pthread_mutex_t printLock;   // Lock for print use

void out(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  lindaTuple newTuple;
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls out" << std::endl;
  pthread_mutex_unlock(&printLock);

  if (generateOutTuple(elems, newTuple, localVars, userDefinedFuncs, loopSymbols, threadNum)) {
    // Use global tuples
    pthread_mutex_lock(&gLock);
    globalTuples.push_back(newTuple);
    pthread_mutex_unlock(&gLock);
    // Notify all waiting threads
    pthread_mutex_lock(&wLock);
    for (std::vector<pthread_cond_t *>::iterator it = waitingList.begin(); it != waitingList.end(); it++) {
      pthread_cond_signal(*it);
    }
    waitingList.clear();
    pthread_mutex_unlock(&wLock);
  }
}

void in(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls in" << std::endl;
  pthread_mutex_unlock(&printLock);
  // Use global tuples
  pthread_mutex_lock(&gLock);
  while (1) {
    std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    if (result == globalTuples.end()) {
      // Didn't find tuples
      pthread_mutex_lock(&printLock);
      std::cout << "Thread " << threadNum << " block from in" << std::endl;
      pthread_mutex_unlock(&printLock);

      // Add thread to wait list
      pthread_mutex_lock(&wLock);
      waitingList.push_back(threadLocks[threadNum]);
      pthread_mutex_unlock(&wLock);
      pthread_cond_wait(threadLocks[threadNum], &gLock);
    } else {
      // Found tuples
      globalTuples.erase(result);
      pthread_mutex_lock(&printLock);
      std::cout << "Thread " << threadNum << " unblock from in" << std::endl;
      pthread_mutex_unlock(&printLock);
      break;
    }
  }
  pthread_mutex_unlock(&gLock);
}

void rd(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls rd" << std::endl;
  pthread_mutex_unlock(&printLock);

  pthread_mutex_lock(&gLock);
  while (1) {
    std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    if (result == globalTuples.end()) {
      // Didn't find tuples
      pthread_mutex_lock(&printLock);
      std::cout << "Thread " << threadNum << " block from rd" << std::endl;
      pthread_mutex_unlock(&printLock);
      pthread_mutex_lock(&wLock);
      waitingList.push_back(threadLocks[threadNum]);
      pthread_mutex_unlock(&wLock);
      pthread_cond_wait(threadLocks[threadNum], &gLock);
    } else {
      break;
    }
  }
  pthread_mutex_unlock(&gLock);
}

bool inp(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls inp" << std::endl;
  pthread_mutex_unlock(&printLock);

  bool r = false;
  pthread_mutex_lock(&gLock);
  std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
  if (result != globalTuples.end()) {
    globalTuples.erase(result);
    r = true;
  }
  pthread_mutex_unlock(&gLock);
  return r;
}

bool rdp(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls rdp" << std::endl;
  pthread_mutex_unlock(&printLock);

  bool result = false;
  pthread_mutex_lock(&gLock);
  if (findInTuple(elems, localVars, userDefinedFuncs, loopSymbols, threadNum) != globalTuples.end())
    result = true;
  pthread_mutex_unlock(&gLock);
  return result;
}

void eval(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls eval" << std::endl;
  pthread_mutex_unlock(&printLock);
  // Preprocess the expression
  for (size_t i = 0; i < elems.size(); i++) {
    if (isExp(elems[i])) {
      // first case: get expression result
      int result = evaluateExp(elems[i], loopSymbols, userDefinedFuncs, localVars, threadNum);
      if (result != -1) {
	elems[i] = std::to_string(static_cast<long long int>(result));
      } else {
	std::cout << "Couldn't evaluate exp " << elems[i] << std::endl;
      }
    } else if (loopSymbols.find(elems[i]) != loopSymbols.end()) {
      //second case: Change loop symbol to value
      elems[i] = std::to_string(static_cast<long long int>(loopSymbols[elems[i]]));
    }
  }
  // Generate tuple and add to global tuples
  lindaTuple newTuple;
  if (generateOutTuple(elems, newTuple, localVars, userDefinedFuncs, loopSymbols, threadNum)) {
    // Use global tuples
    pthread_mutex_lock(&gLock);
    globalTuples.push_back(newTuple);
    pthread_mutex_unlock(&gLock);
    // Notify all waiting threads
    pthread_mutex_lock(&wLock);
    for (std::vector<pthread_cond_t *>::iterator it = waitingList.begin(); it != waitingList.end(); it++) {
      pthread_cond_signal(*it);
    }
    waitingList.clear();
    pthread_mutex_unlock(&wLock);
  }
}

void dump() {
  pthread_mutex_lock(&printLock);
  std::cout << "Dump!" << std::endl;
  pthread_mutex_unlock(&printLock);
  // Access global tuples
  pthread_mutex_lock(&gLock);
  for (std::vector<lindaTuple>::iterator it = globalTuples.begin(); it != globalTuples.end(); it++) {
    std::cout << *it << std::endl;
  }
  pthread_mutex_unlock(&gLock);
}

void lindaFor(int start, int end, std::string symbol, std::vector<std::string>lines, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls for()" << std::endl;
  pthread_mutex_unlock(&printLock);

  for (int i = start; i < end; i++) {
    loopSymbols[symbol] = i;
    // Process loop body
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

void lindaIF(std::vector<std::string>lines, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  pthread_mutex_lock(&printLock);
  std::cout << "Thread " << threadNum << " calls if()" << std::endl;
  pthread_mutex_unlock(&printLock);

  size_t start = (lines[0]).find("(") + 1;
  size_t end = (lines[0]).find_last_of(")");
  std::string condition = (lines[0]).substr(start, end - start);

  // Separate if and else cases
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
  if (evaluateExp(condition, loopSymbols, userDefinedFuncs, localVars, threadNum)) {
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

void runFunc(LINDA_TYPE type, std::vector<std::string> &lines, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  std::vector<std::string> elems;
  int start, end;
  std::string symbol;
  switch (type) {
  case IN:
    getInOutElems(lines[0], elems);
    in(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case OUT:
    getInOutElems(lines[0], elems);
    out(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case EVAL:
    getInOutElems(lines[0], elems);
    eval(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case RD:
    getInOutElems(lines[0], elems);
    rd(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case RDP:
    getInOutElems(lines[0], elems);
    rdp(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case INP:
    getInOutElems(lines[0], elems);
    inp(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case DUMP:
    dump();
    break;
  case IF:
    lindaIF(lines, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case FOR:
    symbol = getForParams(lines[0], &start, &end);
    lindaFor(start, end, symbol, lines, localVars, userDefinedFuncs, loopSymbols, threadNum);
    break;
  case DEFINE:
    writeFile(lines, userDefinedFuncs, threadNum);
    break;
  default:
    std::cout << "Thread " << threadNum << " Unknown: " << lines[0] << std::endl;
  }
}

void *threadProcessor(void *args) {
  // Open file and get thread number
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
  VarMap localVars;
  FunctSet userDefinedFuncs;
  LoopMap loopSymbols;

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

int main(int argc, char** argv) {
  // Read file names
  std::vector<std::string> lines;
  std::string line;
  while (std::cin >> line) {
    lines.push_back(line);
  }

  // Initialize mutex locks
  pthread_mutex_init(&gLock, NULL);
  pthread_mutex_init(&wLock, NULL);
  pthread_mutex_init(&printLock, NULL);

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
    lines[i] = lines[i] + "#" + std::to_string(static_cast<long long int>(i));
    pthread_mutex_lock(&printLock);
    std::cout << "Create thread for file \"" << lines[i] << "\'" << std::endl;
    pthread_mutex_unlock(&printLock);
    //std::cout << "args " << args << std::endl;
    status = pthread_create(allThreads + i, NULL, threadProcessor, (void *)(lines[i].c_str()));
    if (status != 0) {
      pthread_mutex_lock(&printLock);
      std::cout << "Create thread for file \"" << lines[i] << "\" error" << std::endl;
      pthread_mutex_unlock(&printLock);
      exit(EXIT_FAILURE);
    }
  }

  for (int i = 0; i < numThread; i++) {
    pthread_join(allThreads[i], &exit_status);
  }

  pthread_mutex_destroy(&gLock);
  pthread_mutex_destroy(&wLock);
  pthread_mutex_destroy(&printLock);
  for (size_t i = 0; i < threadLocks.size(); i++)
    pthread_cond_destroy(threadLocks[i]);
  return 0;
}
