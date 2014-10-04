#include <string>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "tupleObjects.h"

intObj::intObj(int data) {val = data;}
int intObj::getType() {return INT;}
int intObj::get() const {return val;}

doubleObj::doubleObj(double data) {val = data;}
int doubleObj::getType() {return DOUBLE;}
double doubleObj::get() const {return val;}

stringObj::stringObj(std::string data) {val = data;}
int stringObj::getType() {return STRING;}
std::string stringObj::get() const {return val;}

patternObj::patternObj(std::string data) {val = data;}
int patternObj::getType() {
  if (val[1] == 'i' || val[1] == 'I')
    return INT;
  else if (val[1] == 'd' || val[1] == 'D')
    return DOUBLE;
  else if (val[1] == 's' || val[1] == 'S')
    return STRING;
  else
    return -1;
}
std::string patternObj::get() const {return val;}

// Overload the << operator for output 
std::ostream& operator<<(std::ostream& os, const lindaObj& linda)
{
  const intObj *i;
  const doubleObj *d;
  const stringObj *s;
  if ((i = dynamic_cast<const intObj *>(&linda))) {
    os << i->get();
  } else if ((d = dynamic_cast<const doubleObj *>(&linda))) {
    os << d->get();
  } else if ((s = dynamic_cast<const stringObj *>(&linda))) {
    os << s->get();
  } else {
    std::cout << "No matching type to print" << std::endl;
    exit(EXIT_FAILURE);
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const lindaTuple& linda)
{
  os << "( ";
  for (lindaTuple::const_iterator it = linda.begin(); it != linda.end(); it++) {
    lindaObj *ptr = *it;
    os << *ptr << " ";
  }
  os << ")";
  return os;
}
