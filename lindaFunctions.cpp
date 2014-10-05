#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <string.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <regex>
#include "tupleObjects.h"
#include "tokenizer.h"
#include "lindaFunctions.h"

std::vector<lindaTuple> globalTuples;

void out(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  lindaTuple newTuple;
  if (generateOutTuple(elems, newTuple, localVars, userDefinedFuncs, loopSymbols))
    globalTuples.push_back(newTuple);
}

void in(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  std::vector<lindaTuple>::iterator result = findInTuple(elems, localVars, userDefinedFuncs, loopSymbols);
  if (result == globalTuples.end())
    //block
    std::cout << "block from in" << std::endl;
  else
    globalTuples.erase(result);
}

void rd(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  if (findInTuple(elems, localVars, userDefinedFuncs, loopSymbols) == globalTuples.end())
    //block
    std::cout << "block from rd" << std::endl;
  else
    return;
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

void lindaFor(int start, int end, std::string symbol, std::vector<std::string>lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  //for (auto line:lines) std::cout << line << std::endl;
  for (int i = start; i < end; i++) {
    loopSymbols[symbol] = i;
    for (std::vector<std::string>::iterator it = lines.begin() + 1; it != lines.end(); it++) {
      LINDA_TYPE type = findFunctionType(*it);
      if (isOneLineCommand(type)) {
	std::vector<std::string> newLines(it, it + 1);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
      } else if (isMultiLineCommand(type)) {
	std::vector<std::string> newLines = getMultiLines(lines, it);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
      }
    }
  }
  loopSymbols.erase(symbol);
}

void lindaIF(std::vector<std::string>lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
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
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
      } else if (isMultiLineCommand(type)) {
	std::vector<std::string> newLines = getMultiLines(lines, it);
	//for (auto line:newLines) std::cout << line << std::endl;
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
      }
    }
  } else {
    //std::cout << "false" << std::endl;
    for (std::vector<std::string>::iterator it = elseExps.begin(); it != elseExps.end(); it++) {
      LINDA_TYPE type = findFunctionType(*it);
      if (isOneLineCommand(type)) {
	std::vector<std::string> newLines(it, it + 1);
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
      } else if (isMultiLineCommand(type)) {
	std::vector<std::string> newLines = getMultiLines(lines, it);
	//for (auto line:newLines) std::cout << line << std::endl;
	runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
      }
    }
  }
}


void runFunc(LINDA_TYPE type, std::vector<std::string> &lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols) {
  std::vector<std::string> elems;
  int start, end;
  std::string symbol;
  switch (type) {
  case IN:
    std::cout << "in" << std::endl;
    getInOutElems(lines[0], elems);
    in(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case OUT:
    std::cout << "out" << std::endl;
    getInOutElems(lines[0], elems);
    out(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case EVAL:
    std::cout << "eval" << std::endl;
    getInOutElems(lines[0], elems);
    eval(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case RD:
    std::cout << "rdp" << std::endl;
    getInOutElems(lines[0], elems);
    rd(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case RDP:
    std::cout << "rdp" << std::endl;
    getInOutElems(lines[0], elems);
    rdp(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case INP:
    std::cout << "inp" << std::endl;
    getInOutElems(lines[0], elems);
    inp(elems, localVars, userDefinedFuncs, loopSymbols);
    break;
  case DUMP:
    std::cout << "dump" << std::endl;
    dump();
    break;
  case IF:
    std::cout << "if" << std::endl;
    //for (auto f:lines) std::cout << f << std::endl;
    lindaIF(lines, localVars, userDefinedFuncs, loopSymbols);
    break;
  case FOR:
    std::cout << "for" << std::endl;
    symbol = getForParams(lines[0], &start, &end);
    //std::cout << "Params " << start << end << symbol << std::endl;
    lindaFor(start, end, symbol, lines, localVars, userDefinedFuncs, loopSymbols);
    break;
  case DEFINE:
    std::cout << "define" << std::endl;
    writeFile(lines, userDefinedFuncs, 1); // change 1 to thread number
    //for (auto f:userDefinedFuncs) std::cout << f << std::endl;
    break;
  default:
    std::cout << "other" << std::endl;
  }
}

void *thread_interpreter(std::string &filename) {
  std::vector<lindaTuple> localVars;
  // Open file
  std::ifstream infile(filename.c_str());
  if (!infile) {
    std::cout << "Can not open file " << filename << std::endl;
    exit(EXIT_FAILURE);
  }
  // Read file
  std::string line;
  while (std::getline(infile, line)) {
  }
  dump();
  infile.close();  
  return NULL;
}

int main() {
  std::string filename;
  // Standard output and input
  std::cout << "Enter input filenames: ";
  std::cin >> filename;

  // Open file
  std::ifstream infile(filename.c_str());
  if (!infile) {
    std::cout << "Can not open file " << filename << std::endl;
    exit(EXIT_FAILURE);
  }

  // Build a local variables' map 
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
    LINDA_TYPE type = findFunctionType(*it);
    if (isOneLineCommand(type)) {
      std::vector<std::string> newLines(it, it + 1);
      runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
    } else if (isMultiLineCommand(type)) {
      std::vector<std::string> newLines = getMultiLines(lines, it);
      //for (auto line:newLines) std::cout << line << std::endl;
      runFunc(type, newLines, localVars, userDefinedFuncs, loopSymbols);
    } else {
      if (!line.empty()) {
	std::cout << "Can not recognize command " << std::endl;
	exit(EXIT_FAILURE);
      }
    }
  }
  //  pthread.join();
  return 0;
}
