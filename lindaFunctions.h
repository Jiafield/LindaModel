#ifndef lindaFunctions
#define lindaFunctions

extern std::vector<lindaTuple> globalTuples;

void out(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols);

void in(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum);

void rd(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum);

bool inp(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols);

bool rdp(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols);

void eval(std::vector<std::string> &elems, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols);

void dump();

void lindaFor(int start, int end, std::string symbol, std::vector<std::string>lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum);

void lindaIF(std::vector<std::string>lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum);

void runFunc(LINDA_TYPE type, std::vector<std::string> &lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols, int threadNum);

void *threadProcessor(void *args);
#endif
