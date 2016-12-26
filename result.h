#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <classdesc.h>

/* structure of the resultdb data field */

static const char *classn[]={"once","repeat","nonrepeat","infertile","noninteract"};

class Result 
{
public:
  enum Class {once, repeat, nonrepeat, infertile,noninteract} clas;
  std::string result;
  unsigned firstDiv, copyTime, outMatches, inMatches;
  Result(): clas(noninteract) {firstDiv=copyTime=outMatches=inMatches=0;}
  std::string classDescription() {return classn[clas];}
};

#include "result.cd"

#endif
