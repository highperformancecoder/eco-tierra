#ifndef WORDSIZE_H
#define WORDSIZE_H
/*
  \c Cell_bitsize determines the maximum size of a cell, hence organism \c (=2^Cell_bitsize).
  The maximum number of cells must be \c <= 2^(8*sizeof(Word)-Cell_bitsize)
*/

//typedef long Word;
//typedef unsigned long uWord;
typedef int Word;
typedef unsigned int uWord;
//const unsigned Cell_bitsize=18;
// check against eco-tierra.3
const unsigned Cell_bitsize=9;

struct Template
{
  Word t;
  int size;
  operator Word() const {return t;}
  Template() {}
  Template(Word t,int size): t(t), size(size) {}
};

#include "CPUInst0.h"
#include "CPUInst0.cd"

// select the CPU instruction set to use here.
typedef CPUInst0 CPU;
typedef CPU::instr_set Instr_set;


/**
   a pointer look alike that doesn't own its object
   TODO: move this into the EcoLab library.
*/
template <class T>
class dynamic_ref
{
  T* ref;
public:
  dynamic_ref(T& x): ref(&x) {}
  const dynamic_ref& operator=(T& x) {ref=&x;}
  T& operator*() {return *ref;}
  T* operator->() {return ref;}
  const T& operator*() const {return *ref;}
  const T* operator->() const {return ref;}
};



#endif
