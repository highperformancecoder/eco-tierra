#ifndef VECTORCPU16BIT_H
#define VECTORCPU16BIT_H

#include "Wordsize.h"
#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>

typedef char OpTable[16][16];


struct VectorCPU16bit
{
  static const int numNormalops=14;
  // use uppercase opcodes to avoid C++ keywords
  enum OpCodes {COPY, CMPL, ADDV, CARRY, XOR, AND, SHLV, MAL=numNormalops,DIV};
  enum IncDecFlags {none=0, decsrc, incdst, incpcifz}; 
  typedef uint32_t Instr_set;
  static const uint32_t instr_sz=0xFFFF;
  static const unsigned numRegisters=12;
  Word registers[VectorCPU16bit::numRegisters];
  unsigned cellID;
  bool active;
  unsigned inst_exec;
  int faults, divs, movDaught;
  void execute(Instr_set);

  VectorCPU16bit(unsigned cell=0) {init(cell);}
  void init(unsigned cell) {
    cellID=cell;
  }
  bool sameState(const VectorCPU16bit&) {/* TODO */}
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

  static std::string regName(int);
  
  static void assemble(std::vector<VectorCPU16bit::Instr_set>& code, std::istream& text);
  static void disassemble(std::ostream& text, const std::vector<VectorCPU16bit::Instr_set>& code);

private:
};

// describe instruction opcode layout
union VectorInstr16bit
{
  VectorCPU16bit::Instr_set instr;
  struct
  {
    unsigned incDec:2;
    bool shift:1;
    bool mask0:1;
    unsigned op:4;
    unsigned src: 4;
    unsigned dst: 4;
  };
};


#include "VectorCPU16bit.cd"
#endif
