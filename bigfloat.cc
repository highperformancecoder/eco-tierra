#include "bigfloat.h"
#include <float.h>
#include <math.h>

inline static double min(double x, double y) {return x<y?x:y;}
static const double sqrt_DBL_MAX=sqrt(DBL_MAX), sqrt_DBL_MIN=sqrt(DBL_MIN);
static const int inc=DBL_MAX_EXP/10;
//static const int inc=floor( log( min(sqrt_DBL_MAX,sqrt_DBL_MIN ))/log(32.0));
//static const double pow32inc=pow(32.0,inc), pow32minc=pow(32.0,-inc);
static const double log32ofe=1/log(32.0);
static const double log32of10=1/log10(32.0);

void bigfloat::renormalize()
{
  if (mant > sqrt_DBL_MAX)
    {mant = ldexp(mant,-5*inc); exp+=inc;}
  else if (mant < sqrt_DBL_MIN)
    {mant = ldexp(mant,5*inc); exp-=inc;}
}

bigfloat bigfloat::operator+(bigfloat x)
{
  bigfloat max, min;

  renormalize(); x.renormalize();

  /* shift x so that this and x are in the same order of magnitude */
  int shift = log32ofe * log(mant/x.mant);
  x.mant = ldexp(x.mant,5*shift);
  x.exp-=shift;

  if (exp>x.exp)
    {max=*this; min=x;}
  else
    {min=*this; max=x;}

  /* test firstly if there is any significance to the addition */
  if (max.exp-min.exp < DBL_DIG * log32of10)
    {
      min.mant = ldexp(min.mant, 5*(min.exp-max.exp));  /* shift mantissa */
      max.mant += min.mant;   /* add mantissas */
    }
  return max;
}

bigfloat bigfloat::operator*(bigfloat x)
{
  bigfloat r;
  renormalize(); x.renormalize();
  r.mant = mant * x.mant;
  r.exp  = exp + x.exp;

  return r;
} 

std::ostream& operator<<(std::ostream& s, bigfloat x)
{
  double m1, e1, e2;

  /* express exponent as m1*10^e1 */
  m1 = pow(10, modf(x.exp*log10(32.0),&e1));
  s << pow(10, modf( log10( x.mant * m1), &e2));
  s << "E" << e1+e2;
  return s;
}
