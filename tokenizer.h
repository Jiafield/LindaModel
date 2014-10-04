#ifndef tokenizer
#define tokenizer

typedef enum{IN, OUT, EVAL, RD, RDP, INP, DUMP, IF, ELSE, FOR, DEFINE, EXIST_DEF, OTHER} LINDA_TYPE;

bool isExp(std::string &s);
bool isPattern(std::string &s);
bool isString(std::string &s);
bool isInt(std::string &s);
bool isDouble(std::string &s);

bool isObjectMatch(lindaObj &newO, lindaObj &oldO, std::map<std::string, lindaObj *> &localVars);
bool isMatch(lindaTuple &newT, lindaTuple &oldT, std::map<std::string, lindaObj *> &localVars);
void storeLocalVar(lindaObj &newO, lindaObj &oldO, std::map<std::string, lindaObj *> &localVars);

bool generateOutTuple(std::vector<std::string> &s, lindaTuple &newTuple, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs);

std::vector<lindaTuple>::iterator findInTuple(std::vector<std::string> &s, std::map<std::string, lindaObj *> &localVars, std::set<std::string> &userDefinedFuncs);

LINDA_TYPE findFunctionType(std::string s);
void getInOutElems(std::string s, std::vector<std::string> &elems);

std::string getForParams(std::string s, int *start, int *end);

void split(std::string &s, char delim, std::vector<std::string> &elems);
std::string getFunctName(std::string &line);
void writeFile(std::vector<std::string> &lines, std::set<std::string> &userDefinedFuncs, int threadNum);

std::string generateArgs(std::vector<std::string> &args);
#endif
