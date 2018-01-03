#ifndef VECTORCPU_H
#define VECTORCPU_H

#include "Wordsize.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

typedef char OpTable[16][16];


struct VectorCPU
{
  enum OpCodes {copy, cmpl, addv, carry, mal=0x20,div,adrf};
  static const int numNormalops=4, numSpecialOps=3;
  typedef uint32_t Instr_set;
  static const uint32_t instr_sz=~0U;
  static const unsigned numRegisters=256;
  Word registers[VectorCPU::numRegisters];
  unsigned cellID;
  bool active;
  unsigned inst_exec;
  int faults, divs, movDaught;
  void execute(Instr_set);
  void doSpecialOp(int32_t opCode,Instr_set instr, int32_t src,int32_t& dest);

  VectorCPU(unsigned cell=0) {init(cell);}
  void init(unsigned cell) {
    cellID=cell;
  }
  bool sameState(const VectorCPU&) {/* TODO */}
  Word stackLowWater;
  // special named registers
  Word PC() const {return registers[0];};
  Word PC(Word x) {return registers[0]=x;};
  Word SP() const {return registers[1];};
  /// search for a template at s, looking no further than s+n.
  /// @return zero length template if no template present
  static Template templateAt(Instr_set* s, size_t n) {/* TODO */}
  /// returns true if instr causes an address match to take place
  bool adrMatchInstr(Word instr) {/* TODO */}
  /// returns 1 if fwd jump instr, -1 if backwards, 0 oterwise
  int adrMatchDir(Word instr) {/* TODO */}
private:
};

union VectorInstr
{
  VectorCPU::Instr_set instr;
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

void assemble(std::vector<VectorCPU::Instr_set>& code, std::istream& text);
void disassemble(std::ostream& text, const std::vector<VectorCPU::Instr_set>& code);

#include "VectorCPU.cd"
#endif
