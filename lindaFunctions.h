#ifndef lindaFunctions
#define lindaFunctions

extern std::vector<lindaTuple> globalTuples;

void runFunc(LINDA_TYPE type, std::vector<std::string> &lines, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs, std::map<std::string, int> &loopSymbols);

#endif
