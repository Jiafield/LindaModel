#ifndef tupleObjects
#define tupleObjects

typedef enum {INT, DOUBLE, STRING} OBJ_TYPE;
// Parent class of linda object, for the purpose of using polymorphism.
class lindaObj {
public:
  virtual ~lindaObj() {}
  virtual int getType() {return -1;}
};

// 3 data types inherites from lindaObj
// Each instance of the following object could store a single data(int, double, string, etc)
class intObj : public lindaObj {
private:
  int val;
public:
  intObj(int data);

  virtual int getType();
  
  int get() const;

  virtual ~intObj() {}
};

class doubleObj : public lindaObj {
private:
  double val;
public:
  doubleObj(double data);

  virtual int getType();

  double get() const;

  virtual ~doubleObj() {}
};

class stringObj : public lindaObj {
private:
  std::string val;
public:
  stringObj(std::string data);

  virtual int getType();

  std::string get() const;

  virtual ~stringObj() {}
};

class patternObj : public lindaObj {
 private:
  std::string val;
 public:
  patternObj(std::string data);
  virtual int getType();
  std::string get() const;
  virtual ~patternObj() {}
};

// Inheritant from a vector of lindaObj pointers.
// Each instance of the following object store a tuple.
// This is the element of the global tuples
class lindaTuple : public std::vector<lindaObj *> {

};

// Overload the << operator for output 
template <class T>
std::ostream& operator<<(std::ostream& os, const lindaObj& linda);
std::ostream& operator<<(std::ostream& os, const lindaTuple& linda);

#endif
