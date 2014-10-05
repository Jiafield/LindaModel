#ifndef tokenizer
#define tokenizer

#include "lindaFunctions.h"

bool isExp(std::string &s);
bool isPattern(std::string &s);
bool isString(std::string &s);
bool isInt(std::string &s);
bool isDouble(std::string &s);

bool isObjectMatch(lindaObj &newO, lindaObj &oldO, VarMap &localVars);
bool isMatch(lindaTuple &newT, lindaTuple &oldT, VarMap &localVars);
void storeLocalVar(lindaObj &newO, lindaObj &oldO, VarMap &localVars);

bool generateOutTuple(std::vector<std::string> &s, lindaTuple &newTuple, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

std::vector<lindaTuple>::iterator findInTuple(std::vector<std::string> &s, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

LINDA_TYPE findFunctionType(std::string s);
bool isOneLineCommand(LINDA_TYPE type);
bool isMultiLineCommand(LINDA_TYPE type);
std::vector<std::string> getMultiLines(std::vector<std::string> &lines, std::vector<std::string>::iterator &it);
int evaluateExp(std::string expr, LoopMap &loopSymbols, FunctSet &userDefinedFuncs, VarMap &localVars, int threadNum);
void getInOutElems(std::string s, std::vector<std::string> &elems);

std::string getForParams(std::string s, int *start, int *end);

void split(std::string &s, char delim, std::vector<std::string> &elems);
std::string getFunctName(std::string &line);
void writeFile(std::vector<std::string> &lines, FunctSet &userDefinedFuncs, int threadNum);

std::string generateArgs(std::vector<std::string> &args);
#endif
