#include <vector>
#include <map>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <regex>
#include <locale>
#include <algorithm>
#include "tupleObjects.h"
#include "tokenizer.h"
#include "lindaFunctions.h"

bool isExp(std::string &s) {
  std::regex rx(".*\\(.*\\)");
  return std::regex_match(s, rx);
}

bool isPattern(std::string &s) {
  return s[0] == '?';
}

bool isVarPattern(std::string &s) {
  return s[0] == 'd' || s[0] == 's' || s[0] == 'i';
}

bool isString(std::string &s) {
  return (s[0] == '"') and (s[s.length() - 1] == '"');
}

bool isInt(std::string &s) {
  return s.find_first_not_of("0123456789") == std::string::npos;
}

bool isDouble(std::string &s) {
  return s.find_first_not_of("0123456789.") == std::string::npos;
}

bool isObjectMatch(lindaObj &newO, lindaObj &oldO, VarMap &localVars) {
  // Variable shows the type it represent here.
  if (newO.getType() != oldO.getType())
    return false;
  if ((dynamic_cast<patternObj *>(&newO)))
    return true;
  else {
    if (oldO.getType() == INT) {
      intObj *i1 = dynamic_cast<intObj *>(&oldO);
      intObj *i2 = dynamic_cast<intObj *>(&newO);
      return i1->get() == i2->get();
    } else if (oldO.getType() == DOUBLE) {
      doubleObj *d1 = dynamic_cast<doubleObj *>(&oldO);
      doubleObj *d2 = dynamic_cast<doubleObj *>(&newO);      
      return d1->get() == d2->get();
    } else {
      stringObj *s1 = dynamic_cast<stringObj *>(&oldO);
      stringObj *s2 = dynamic_cast<stringObj *>(&oldO);
      return s1->get() == s2->get();
    }
  }
  return false;
}

bool isMatch(lindaTuple &newT, lindaTuple &oldT, VarMap &localVars) {
  if (newT.size() != oldT.size())
    return false;
  for (size_t i = 0; i < newT.size(); i++) {
    if (!isObjectMatch(*(newT[i]), *(oldT[i]), localVars))
      return false;
  }
  // Match the tuple, store local varibles
  for (size_t i = 0; i < newT.size(); i++) {
    storeLocalVar(*(newT[i]), *(oldT[i]), localVars);
  }  
  return true;
}

void storeLocalVar(lindaObj &newO, lindaObj &oldO, VarMap &localVars) {
  patternObj *p;
  if ((p = dynamic_cast<patternObj *>(&newO))) {
    std::string varName = (p->get()).replace(0, 1, "");
    if (oldO.getType() == INT) {
      intObj *i = dynamic_cast<intObj *>(&oldO);
      localVars[varName] = new intObj(i->get());
    } else if (oldO.getType() == DOUBLE) {
      doubleObj *d = dynamic_cast<doubleObj *>(&oldO);
      localVars[varName] = new doubleObj(d->get());
    } else {
      stringObj *s = dynamic_cast<stringObj *>(&oldO);
      localVars[varName] = new stringObj(s->get());
    }
  }
}

bool generateOutTuple(std::vector<std::string> &s, lindaTuple &newTuple, 
		      VarMap &localVars, 
		      FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  for (std::vector<std::string>::iterator it = s.begin(); it != s.end(); it++) {
    if (isExp(*it)) {
      int result = evaluateExp(*it, loopSymbols, userDefinedFuncs, localVars, threadNum);
      if (result != -1) {
	newTuple.push_back(new intObj(result));
      } else {
	// wait and block
      }
    } else if (isVarPattern(*it)) {
      if (localVars.find(*it) != localVars.end()) {
	newTuple.push_back(localVars[*it]);
      } else {
	std::cout<< "Can not find local var " << *it << std::endl;
	return false;
      }
    }
    else if (isString(*it))
      newTuple.push_back(new stringObj(*it));
    else if (isInt(*it))
      newTuple.push_back(new intObj(atoi(it->c_str())));
    else if (isDouble(*it))
      newTuple.push_back(new doubleObj(atof(it->c_str())));
    else {
      std::cout << "Parse input error: " << *it << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  return true;
}

std::vector<lindaTuple>::iterator findInTuple(std::vector<std::string> &s, 
					      VarMap &localVars, 
					      FunctSet &userDefinedFuncs, LoopMap &loopSymbols, int threadNum) {
  // Build a tuple for the input.
  lindaTuple newTuple;
  for (std::vector<std::string>::iterator it = s.begin(); it != s.end(); it++) {
    //std::cout << "elems " << *it << std::endl;
    if (isExp(*it)) {
      int result = evaluateExp(*it, loopSymbols, userDefinedFuncs, localVars, threadNum);
      if (result != -1) {
	newTuple.push_back(new intObj(result));
      } else {
	// wait and block
      }
    } else if (isPattern(*it)) {
      //std::cout << "pattern" << std::endl;
      newTuple.push_back(new patternObj(*it));
    } else if (isString(*it))
      newTuple.push_back(new stringObj(*it));
    else if (isInt(*it))
      newTuple.push_back(new intObj(atoi(it->c_str())));
    else if (isDouble(*it))
      newTuple.push_back(new doubleObj(atof(it->c_str())));
    else {
      std::cout << "Parse input error" << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  
  // Look up the tuple in the global tuple space
  for (std::vector<lindaTuple>::iterator it = globalTuples.begin(); it != globalTuples.end(); it++) {
    // Overload == operator for the lindaTuple class
    if (isMatch(newTuple, *it, localVars))
      return it;
  }

  // After iterate all global tuple, no found
  return globalTuples.end();
}

LINDA_TYPE findFunctionType(std::string s) {
  // change string to all lower case, change \t to space
  std::locale loc;
  for (unsigned int i = 0; i < s.length(); i++) {
    s[i] = std::tolower(s[i], loc);
    if (s[i] == '\t')
      s[i] = ' ';
  }

  std::regex rIn(" *in *\\(.*\\).*");
  std::regex rOut(" *out *\\(.*\\).*");
  std::regex rEval(" *eval *\\(.*\\).*");
  std::regex rRd(" *rd *\\(.*\\).*");
  std::regex rRdp(" *rdp *\\(.*\\).*");
  std::regex rInp(" *inp *\\(.*\\).*");
  std::regex rDump(" *dump *\\(.*\\).*");

  std::regex rIf(" *if.*");
  std::regex rFor(" *for.*");
  std::regex rDefine(" *define.*");
  if (std::regex_match(s, rIn))
    return IN;
  else if (std::regex_match(s, rOut))
    return OUT;
  else if (std::regex_match(s, rEval))
    return EVAL;
  else if (std::regex_match(s, rRd))
    return RD;
  else if (std::regex_match(s, rRdp))
    return RDP;
  else if (std::regex_match(s, rInp))
    return INP;
  else if (std::regex_match(s, rDump))
    return DUMP;
  else if (std::regex_match(s, rIf))
    return IF;
  else if (std::regex_match(s, rFor))
    return FOR;
  else if (std::regex_match(s, rDefine))
    return DEFINE;
  return OTHER;
}

bool isOneLineCommand(LINDA_TYPE type) {
  return type >= IN and type <=DUMP;
}

bool isMultiLineCommand(LINDA_TYPE type) {
  return type >= IF and type <= DEFINE;
}

std::vector<std::string> getMultiLines(std::vector<std::string> &lines, std::vector<std::string>::iterator &it) {
  std::vector<std::string> newLines;
  int braceCount = std::count((*it).begin(), (*it).end(), '{');
  braceCount -= std::count((*it).begin(), (*it).end(), '}');
  newLines.push_back(*it);
  // In case { not in the same line with if / for
  if ((++it) != lines.end()) {
    braceCount += std::count((*it).begin(), (*it).end(), '{');
    braceCount -= std::count((*it).begin(), (*it).end(), '}');
    newLines.push_back(*it);
  }
  while (braceCount > 0 && (++it) != lines.end()) {
    braceCount += std::count((*it).begin(), (*it).end(), '{');
    braceCount -= std::count((*it).begin(), (*it).end(), '}');
    newLines.push_back(*it);
  }
  // Special for if - else command
  std::vector<std::string>::iterator nextIt = it + 1;
  if (nextIt != lines.end() && (*nextIt).find("else") != std::string::npos) {
    it++;
    braceCount += std::count((*it).begin(), (*it).end(), '{');
    braceCount -= std::count((*it).begin(), (*it).end(), '}');
    newLines.push_back(*it);
    // In case { not in the same line with else
    if ((++it) != lines.end()) { 
      braceCount += std::count((*it).begin(), (*it).end(), '{');
      braceCount -= std::count((*it).begin(), (*it).end(), '}');
      newLines.push_back(*it);
    }
    while (braceCount > 0 && (++it) != lines.end()) {
      braceCount += std::count((*it).begin(), (*it).end(), '{');
      braceCount -= std::count((*it).begin(), (*it).end(), '}');
      newLines.push_back(*it);
    }
  }
    
  if (braceCount != 0) {
    std::cout << "User defined function's {} couldn't match." <<std::endl;
    exit(EXIT_FAILURE);
  }
  return newLines;
}

int evaluateExp(std::string expr, LoopMap &loopSymbols, FunctSet &userDefinedFuncs, VarMap &localVars, int threadNum) {
  int result = -1;
  std::string expName = (expr).substr(0, (expr).find("("));
  std::string expNameT = expName + std::to_string(threadNum);
  size_t start = (expr).find("(") + 1;
  size_t end = (expr).find_last_of(")");
  std::string params = (expr).substr(start, end - start);

  if (userDefinedFuncs.find(expNameT) != userDefinedFuncs.end()) {
    // Case 1: The expression is a user defined function
    if (!isInt(params)) {
      if (loopSymbols.find(params) != loopSymbols.end()) {
	params = std::to_string(loopSymbols[params]);
      } else if (localVars.find(params) != localVars.end()) {
	intObj *intO = dynamic_cast<intObj *>(localVars[params]);
	params = std::to_string(intO->get());
      }
    }
    int status = system(("./" + expNameT + " " + params).c_str());
    result = WEXITSTATUS(status);
  } else {
    // Case 2: The expression is inp or rdp
    //std::cout << "condition is " << expr << std::endl;
    LINDA_TYPE type = findFunctionType(expr);
    //std::cout << "expr type is " << type << std::endl;
    std::vector<std::string> elems;
    if (type == INP) {
      getInOutElems(expr, elems);
      return inp(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    } else if (type == RDP) {
      getInOutElems(expr, elems);
      return rdp(elems, localVars, userDefinedFuncs, loopSymbols, threadNum);
    } else {
      std::cout << "Couldn't evaluate expression " << expr << std::endl;
      std::cout << "No boolean return type or couldn't find expr name" << std::endl;
      exit(EXIT_FAILURE);
    } 
  }
  return result;
}

void getInOutElems(std::string s, std::vector<std::string> &elems) {
  std::size_t start = s.find_first_of("(");
  std::size_t end = s.find_last_of(")");
  std::string str = s.substr(start + 1, end - start - 1);
  //std::cout << str << std::endl;
  // escape case not covered yet
  split(str, ',', elems);
}

std::string getForParams(std::string s, int *start, int *end) {
  std::size_t begin = s.find_first_of("(");
  std::size_t finish = s.find_last_of(")");
  std::string str = s.substr(begin + 1, finish - begin - 1);
 
  str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());

  std::vector<std::string> elems;
  split(str, ';', elems);
  if (elems.size() != 3) {
    std::cout << "Wrong number arguments of for():" << std::endl;
    std::cout << str << std::endl;
    exit(EXIT_FAILURE);
  }
  //for (auto e:elems)  std::cout << e << std::endl;
  // find start, end and symbol
  std::string str1 = elems[0];
  *start = std::atoi(str1.substr(str1.find_first_of("0123456789")).c_str());

  std::string str2 = elems[1].substr(elems[1].find_first_of("0123456789"));
  *end = std::atoi(str2.c_str());
  if (elems[1].find("=") != std::string::npos)
    *end += 1;

  std::string str3 = elems[2];
  str3.erase(std::remove(str3.begin(), str3.end(), '+'), str3.end());
  return str3;
}

void split(std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    std::size_t pos = item.find_first_not_of(" \t");
    std::size_t pos2 = item.find_last_not_of(" \t");
    item = item.substr(pos, pos2 - pos + 1);
    elems.push_back(item);
    //std::cout << item << std::endl;
  }
}

std::string getFunctName(std::string &line) {
  std::size_t start = line.find("int") + 3;
  std::size_t end = line.find("(");
  if (start != std::string::npos && end != std::string::npos)
    return line.substr(start, end - start);
  return "";
}

void writeFile(std::vector<std::string> &lines, FunctSet &userDefinedFuncs, int threadNum) {
  //std::cout << "Inside writeFile" << std::endl;
  // map filename to the user defined functions
  std::string startLine = lines[0];
  //std::cout << "line 0" << startLine << std::endl;
  std::string fileName = "tempFile" + std::to_string(threadNum) + std::to_string(userDefinedFuncs.size()) + ".cpp";
  std::string funcName = getFunctName(startLine) + std::to_string(threadNum);
  funcName.erase(std::remove(funcName.begin(), funcName.end(), ' '), funcName.end());
  if (funcName.empty()) {
    std::cout << "Read user defined function name error" << std::endl;
    exit(EXIT_FAILURE);
  }
  //std::cout << "write funct name " << funcName << std::endl;
  // write function to a new temp file
  std::ofstream outfile(fileName);
  if (!outfile.is_open()) {
    std::cout << "Can not create file to write user defined functions" <<std::endl;
    exit(EXIT_FAILURE);
  }

  // write first line
  std::stringstream ss;
  ss << "#include <stdlib.h>\n";
  ss << "#include <string>\n";
  ss << "using namespace std;";
  ss << "int main(int argc, char** argv)";
  std::size_t pos = startLine.find_last_of(")");
  ss << startLine.substr(pos + 1) << std::endl;
  // write other
  for (std::vector<std::string>::iterator line = lines.begin() + 1; line != lines.end(); line++) {
    ss << *line << std::endl;
  }
  // Pass arguments to user defined functions
  std::string out = ss.str();
  std::vector<std::string> args;
  getInOutElems(startLine, args);
  std::string argStr = generateArgs(args);
  // Insert parameter to main function
  pos = out.find("{");
  out.insert(pos + 1, argStr);
  outfile << out;
  outfile.close();
  // Compile the user defined function.
  std::string command = "g++ -o " + funcName + " " + fileName;
  //std::cout << command << std::endl;
  system(command.c_str());
  userDefinedFuncs.insert(funcName);
}

// Only support int, double, string as user defined function's parameters.
std::string generateArgs(std::vector<std::string> &args) {
  std::string argStr = "";
  std::regex intR("int .*");
  std::regex doubleR("double .*");
  std::regex stringR("string .*");
  for (size_t i = 0; i < args.size(); i++) {
    if (std::regex_match(args[i],intR)) {
      argStr = argStr + "\n\t" + args[i] + " = atoi(argv[" + std::to_string(i + 1) + "])"+ ";";
    } else if (std::regex_match(args[i], doubleR)) {
      argStr = argStr + "\n\t" + args[i] + " = atof(argv[" + std::to_string(i + 1) + "])"+ ";";      
    } else if (std::regex_match(args[i], stringR)) {
      argStr = argStr + "\n\t" + args[i] + "(argv[" + std::to_string(i + 1) + "])"+ ";";
    } else {
      std::cout << "Couldn't recognize user defined function parameter." << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  return argStr;
}
