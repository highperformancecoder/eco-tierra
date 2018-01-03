#ifndef VECTORCPU_H
#define VECTORCPU_H

#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

typedef char OpTable[16][16];


struct VectorCPU
{
  enum OpCodes {copy, cmpl, addv, carry, mal=0x20,div,adrf};
  static const int numNormalops=4, numSpecialOps=3;
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

union VectorInstr
{
  VectorCPU::instr_set instr;
  struct
  {
    bool decrSrc:1;
    bool incrDst:1;
    unsigned shift:2;
    unsigned mask:4;
    unsigned op:6;
    bool dstInd: 1;
    unsigned dst: 8;
    bool srcInd: 1;
    unsigned src: 8;
  };
  int notNop: 24; // indicates if instruction isn't a "nop"
};

void assemble(std::vector<VectorCPU::instr_set>& code, std::istream& text);
void disassemble(std::ostream& text, const std::vector<VectorCPU::instr_set>& code);

#include "VectorCPU.cd"
#endif
