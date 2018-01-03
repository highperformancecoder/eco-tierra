#include "Soup.h"
#include <ecolab_epilogue.h>

//table of opcode mnemonics
const char* mnemonic[]=
  {
    "nop0","nop1","not0","shl","zero","ifz","subCAB","subAAC","incA","incB",
    "decC","incC","pushA","pushB","pushC","pushD","popA","popB","popC","popD",
    "jmpo","jmpb","call","ret","movDC","movBA","movii","adro","adrb","adrf",
    "mal","divide"
  };

std::ostream& operator<<(std::ostream& o, CPUInst0::instr_set instr)
{
  if (instr>=CPUInst0::nop0 && instr<=CPUInst0::divide)
    return o<<mnemonic[instr];
  else
    return o<<"???";
}

// reference to the global soup
extern Soup<CPUInst0>& soup;

size_t CPUInst0::maxStackSz=100000000;
const size_t CPUInst0::stackSz;

void CPUInst0::prInstr(Instr_set instr)

{
  std::cout << inst_exec<<":PC="<<m_PC <<":"<<classdesc::enum_keys<Instr_set>()(instr) << std::endl;
}

int CPUInst0::moviiImpl(Word AX, Word BX)
{
  // if AX does not point within daughter, bail
  if ((AX&mask) != (daughter&mask))
    return true;
  return soup.movii(AX,BX);
}

void CPUInst0::adr(Word& reg, Word& template_sz, int dir)
{
  //  PC%=soup.memSz();
  Word a=soup.adr(m_PC,template_sz,dir);
  Word prevPC=m_PC;
  if (template_sz<8*sizeof(Word))
    m_PC+=template_sz+1;
  if (a>=0) 
      reg=a;
  else
    faults++;
  if (m_PC==prevPC) m_PC++; //avoid getting stuck in a loop
}

Word CPUInst0::malImpl(Word size)
{
  try
    {
      return soup.mal(size, myCellID);
    }
  catch (std::exception&) // various exceptions thrown if size is invalid
    {
      faults++;
      return 0;
    }
}

bool CPUInst0::divideImpl(Word cell)
{
  return soup.divide(cell);
}

bool CPUInst0::sameState(const CPUInst0& other)
{
  bool r = AX==other.AX && BX==other.BX && CX==other.CX && DX==other.DX &&
    m_PC%soup.memSz() == other.m_PC%soup.memSz();
  if (r)
    {
      int offs=other.m_SP-m_SP;
      for (int k=stackLowWater; k<m_SP; k++) 
        if (stack[k&(stackSz-1)]!=other.stack[(k+offs)&(stackSz-1)])
          return false;
    }
  return r;
}

Template CPUInst0::templateAt(instr_set* s, size_t n)
{
  if (*s > CPU::nop1)
    return Template{-1,0};
  Template t{1,0};
  instr_set* end=s+n;
  for (auto i=s; i<end && *s <= nop1; ++s)
    {
      t.t<<=1;
      t.t|=!*s;
      t.size++;
    }
  return t;
}

