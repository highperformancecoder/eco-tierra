#ifndef CPUInst0_H
#define CPUInst0_H
#include <vector>
#include <map>
#include "pack_base.h"
#include <classdesc_access.h>

// this would be better a static class member of CPUInst0, but a bug
// in classdesc prevents this from being possible.
extern const char* mnemonic[];

/* This class relies upon the integral type Word being defined before
   this class is included */

class CPUInst0
{
public:
  static size_t maxStackSz;

  static const size_t stackSz=16; // needs to be power of two
  //  std::deque<Word> stack;
  Word stack[CPUInst0::stackSz];
  Word stackLowWater;

  /* instruction set */
  enum instr_set {nop0,nop1,not0,shl,zero,ifz,subCAB,subAAC,incA,incB,decC,incC,
                 pushA,pushB,pushC,pushD,popA,popB,popC,popD,jmpo,jmpb,
                 call,ret,movDC,movBA,movii,adro,adrb,adrf,mal,divide,instr_sz};

  /* instr_sz is one plus the highest instruction. Its numerical value
     gives the number of instructions in the instruction set */
  typedef enum instr_set Instr_set;  

  bool active;
  unsigned inst_exec;
  int faults, divs, movDaught;
  Word AX,BX,CX,DX,daughter;
  Word PC() const {return m_PC;}
  Word PC(Word x) {return m_PC=x;}
  Word SP() const {return m_SP;}
  
  CPUInst0(unsigned cell=0) {init(cell);}

  void init(unsigned cell) {
    active=false;
    AX=BX=CX=DX=m_SP=0;
    m_PC=cell<<Cell_bitsize;
    daughter=-1;
    myCellID=cell;
    faults=divs=inst_exec=0;
    movDaught=0;
    for (int i=0; i<stackSz; i++) stack[i]=0;
  }

  /// trace print executions
  void prInstr(Instr_set instr);

  inline void execute(Instr_set instr);

  // true if registers and stack are all equal
  bool sameState(const CPUInst0&);

  /// returns true if instr causes an address match to take place
  bool adrMatchInstr(Word instr) {
    const static Word adrI= (1<<jmpo) | (1<<jmpb) | (1<<call) | 
      (1<<adro) | (1<<adrb) | (1<<adrf);
    return (1<<instr) & adrI;
  }
  // returns 1 if fwd jump instr, -1 if backwards, 0 oterwise
  int adrMatchDir(Word instr) {
    const static Word fwd = (1<<adrf), bwd = (1<<jmpb) | (1<<adrb);
    Word i=(1<<instr);
    return int((i&fwd)!=0) - int((i&bwd)!=0);
  }

  /// search for a template at s, looking no further than s+n.
  /// @return zero length template if no template present
  static Template templateAt(instr_set* s, size_t n);
  
private:
  CLASSDESC_ACCESS(CPUInst0);
  unsigned myCellID;
  Word m_PC, m_SP;
  
  void push(Word reg) {stack[m_SP++&(stackSz-1)]=reg;}
  void pop(Word& reg) {
    reg=stack[--m_SP&(stackSz-1)];
    if (m_SP<stackLowWater) stackLowWater=m_SP;
  }



  int moviiImpl(Word AX, Word BX);
  void adr(Word& PC, Word& template_sz, int dummy); 
  Word malImpl(Word size);
  bool divideImpl(Word cell); 

};

inline void CPUInst0::execute(Instr_set instr)
{
  Word sz, tmpPC;

  //prInstr(instr);
  switch (instr)
    {
    case nop0: case nop1: m_PC++; break;
    case pushA: push(AX); m_PC++; break;
    case pushB: push(BX); m_PC++; break;
    case pushC: push(CX); m_PC++; break;
    case pushD: push(DX); m_PC++; break;
    case popA: pop(AX); m_PC++; break;
    case popB: pop(BX); m_PC++; break;
    case popC: pop(CX); m_PC++; break;
    case popD: pop(DX); m_PC++; break;

    case movDC: DX=CX; m_PC++; break;
    case movBA: BX=AX; m_PC++; break;
    case movii: 
      if (moviiImpl(AX,BX))
        faults++;
      else
        movDaught++;
    m_PC++; break;
    case subCAB: CX=AX-BX; m_PC++; break;
    case subAAC: AX=AX-CX; m_PC++; break;
    case incA: AX++; m_PC++; break;
    case incB: BX++; m_PC++; break;
    case incC: CX++; m_PC++; break;
    case decC: CX--; m_PC++; break;
    case zero: CX=0; m_PC++; break;
    case not0: CX |= !(CX&1); m_PC++; break;
    case shl: CX<<=1; m_PC++; break;
    case ifz: if (CX!=0) m_PC++; m_PC++; break;

      /* if PC is unchanged in this call (ie no match) then increment
         PC to prevent infinite recursion */
    case call: push(m_PC); 
    case jmpo: /*tmpPC=PC;*/ adr(m_PC,sz,0); /*if (tmpPC==PC) {PC+=sz+1; faults++;}*/ break;
    case jmpb: /*tmpPC=PC;*/ adr(m_PC,sz,-1); /*if (tmpPC==PC) {PC+=sz+1; faults++;}*/ break;
    case ret: pop(m_PC); m_PC++; break;

    case adro: adr(AX,CX,0); /*PC+=CX+1;*/ break;
    case adrb: adr(AX,CX,-1); /*PC+=CX+1;*/ break;
    case adrf: adr(AX,CX,1); /*PC+=CX+1;*/ break;
    case mal:  AX=daughter=malImpl(CX); m_PC++; break;
    case divide: 
      if (daughter>=0 && divideImpl(daughter)) 
        divs++;
      else
        faults++;
      m_PC++; 
      break; 
    }

  inst_exec++;
}


std::ostream& operator<<(std::ostream& o, CPUInst0::instr_set instr);

#endif
