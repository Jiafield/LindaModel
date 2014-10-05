#ifndef lindaFunctions
#define lindaFunctions

#include <vector>
#include <string>
#include <map>
#include "tupleObjects.h"

extern std::vector<lindaTuple> globalTuples;
typedef std::map<std::string, lindaObj *> VarMap;
typedef std::map<std::string, int> LoopMap;
typedef std::set<std::string> FunctSet;
typedef enum{IN, OUT, EVAL, RD, RDP, INP, DUMP, IF, FOR, DEFINE, OTHER} LINDA_TYPE;

void out(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols);

void in(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

void rd(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

bool inp(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols);

bool rdp(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols);

void eval(std::vector<std::string> &elems, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols);

void dump();

void lindaFor(int start, int end, std::string symbol, std::vector<std::string>lines, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

void lindaIF(std::vector<std::string>lines, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

void runFunc(LINDA_TYPE type, std::vector<std::string> &lines, VarMap &localVars, FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum);

void *threadProcessor(void *args);
#endif
