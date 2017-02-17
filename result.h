#ifndef RESULT_H
#define RESULT_H
#include "oname.h"

/* structure of the resultdb data field */
enum clas_t {once, repeat, nonrepeat, infertile,noninteract};

static const char *classn[]={"once","repeat","nonrepeat","infertile","noninteract"};

class resultd_t 
{
public:
  enum clas_t  clas;
  oname result;
  float sigma,tau,mu,nu;
  resultd_t() {sigma=tau=mu=nu=0; clas=noninteract; result="";}
};

#include "result.cd"

#endif
