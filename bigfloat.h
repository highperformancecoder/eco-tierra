/* numbers stored internally as mant * 32^exp */
#include <iostream>
#include <math.h>

#ifndef CLASSDESC_ACCESS
#define CLASSDESC_ACCESS(x)
#endif

class bigfloat
{
  CLASSDESC_ACCESS(bigfloat);
  double mant;
  int exp;
  friend double log32(bigfloat);
  friend std::ostream& operator<<(std::ostream&, bigfloat);
  void renormalize();
public:
  bigfloat(double m=0, int e=0): mant(m),exp(e) {}
  bigfloat operator+(bigfloat x);
  bigfloat operator*(bigfloat x);
  bigfloat operator/(bigfloat x){return operator*(bigfloat(1/x.mant,-x.exp));}
  bigfloat operator-() {return bigfloat(-mant,exp);}
  bigfloat operator-(bigfloat x) {x.mant=-x.mant; return operator+(x);}
  bigfloat operator+=(bigfloat x) {return *this=*this+x;}
  bigfloat operator-=(bigfloat x) {return *this=*this-x;}
  bigfloat operator*=(bigfloat x) {return *this=*this*x;}
  bigfloat operator/=(bigfloat x) {return *this=*this/x;}
  int operator<=(bigfloat x) { return (exp==x.exp)? mant<=x.mant: exp<=x.exp;}
};

inline bigfloat operator+(double x, bigfloat y) {return bigfloat(x)+y;}
inline bigfloat operator-(double x, bigfloat y) {return bigfloat(x)-y;}
inline bigfloat operator*(double x, bigfloat y) {return bigfloat(x)*y;}
inline bigfloat operator/(double x, bigfloat y) {return bigfloat(x)/y;}

inline bigfloat pow32(int i) {return bigfloat(1,i);}
inline double log32(bigfloat x) {return x.exp+log(x.mant)/log(32.0);}
inline double log32(double x) {return log(x)/log(32.0);}
std::ostream& operator<<(std::ostream& s, bigfloat x);


