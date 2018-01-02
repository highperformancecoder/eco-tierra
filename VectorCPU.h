#ifndef VECTORCPU_H
#define VECTORCPU_H

#include <stdint.h>

typedef char OpTable[16][16];

struct VectorCPU
{
  typedef uint32_t instr_set;
  static const unsigned numRegisters=256;
  int32_t registers[VectorCPU::numRegisters];
  unsigned cellID;
  void execute(instr_set);
  void doSpecialOp(int32_t opCode,instr_set instr, int32_t src,int32_t& dest);

  VectorCPU(unsigned cell=0) {init(cell);}
  void init(unsigned cell) {
    cellID=cell;
  }
private:
};

#include "VectorCPU.cd"
#endif
