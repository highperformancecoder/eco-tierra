#include "VectorCPU16bit.h"
#include "Soup.h"
#include <ecolab_epilogue.h>
#include <map>
#include <regex>
#include <iomanip>
#include <iostream>
#include <string>
using std::vector;
using std::map;
using std::string;
using std::istream;
using std::ostream;
using std::cerr;
using std::cout;
using std::endl;
using std::regex;
using std::smatch;

namespace
{
  constexpr size_t numOps=7;
  OpTable optable[numOps];
  int initOpTable()
  {
    for (int i=0; i<16; ++i)
      for (int j=0; j<16; ++j)
        {
          optable[0][i][j] = j; //copy
          optable[1][i][j] = ~j; //cmpl
          optable[2][i][j] = (i+j)&0xff; //addv
          optable[3][i][j] = i+j > 0xff; //carry
          optable[4][i][j] = i^j; //xor
          optable[5][i][j] = i&j; //and
          optable[6][i][j] = (j<<1)&0xff; //shlv
        }
    return 1;
  }
  int dum_op=initOpTable();
}


const int VectorCPU16bit::numNormalops;
const uint32_t VectorCPU16bit::instr_sz;
const unsigned VectorCPU16bit::numRegisters;

// reference to the global soup
extern Soup<VectorCPU16bit>& soup;

void VectorCPU16bit::execute(Instr_set instr)
{
  VectorInstr16bit vi; vi.instr=instr;
  cout << classdesc::enumKey<OpCodes>(vi.op)<<" "<<vi.src<<","<<vi.dst<<endl;
  int32_t src, dst;
  unsigned char *srcB=reinterpret_cast<unsigned char*>(&src), *dstB=reinterpret_cast<unsigned char*>(&dst);
  if (vi.src<numRegisters)
    src=registers[vi.src];
  else
    src=soup.get(registers[vi.src-numRegisters]); //indirect addressing
  if (vi.dst<12)
    dst=registers[vi.dst];
  else
    dst=soup.get(registers[vi.dst-numRegisters]); //indirect addressing

  if (vi.op<numNormalops)
    {
      int32_t mask=vi.mask0? 0xFFU: 0xFFFFFFFFU;
      OpTable& op=optable[vi.op % numOps];
      dst=mask&
        (op[dstB[0]][srcB[0]] |
         (op[dstB[1]][srcB[1]]<<8) |
         (op[dstB[2]][srcB[2]]<<16) |
         (op[dstB[3]][srcB[3]]<<24)) |
        ~mask&dst;
    }        
  else
    switch (vi.op)
      {
      case int(MAL):
        dst=soup.mal(src,cellID);
        return;
      case int(DIV):
        soup.divide(cellID);
        return;
      }
  if (vi.shift) (dst<<8)|(dst>>24); //apply shift bits  
  if (vi.dst<numRegisters)
    registers[vi.dst]=dst;
  else
    soup.set(registers[vi.dst-numRegisters],dst);
  // apply increment/decrement flags
  switch (IncDecFlags(vi.incDec))
    {
    case none: break;
    case decsrc: registers[vi.src]--; break;
    case incdst: registers[vi.dst]++; break;
    case incpcifz: if (!dst) registers[0]++; break;
    }
  
  // increment PC
  registers[0]++;
}

namespace
{
  int parseRegister(const string& reg)
  {
    if (reg=="pc")
      return 0;
    else if (reg.length()>1)
      return stoi(reg.substr(1));
  }
}

void VectorCPU16bit::assemble(vector<Instr_set>& code, istream& text)
{
  string buf;
  regex comment("^[[:space:]]*;.*|^[[:space:]]*$");
  regex shift("cshift.*:.*"), mask("mask0.*:.*");
  regex qualifiedOp(R"(.*:[[:space:]]*([[:alnum:]]*)[[:space:]]+(\[?)([[:alnum:]]*)\]?,(\[?)([[:alnum:]]*).*)");
  regex bareOp(R"(^[[:space:]]*([[:alnum:]]*)[[:space:]]+(\[?)([[:alnum:]]*)\]?,(\[?)([[:alnum:]]*).*)");
  smatch match;
  while (getline(text,buf))
    {
      for (auto& i: buf) i=tolower(i);
      if (regex_match(buf,comment)) continue;
      VectorInstr16bit c;
      c.instr=0;
      int flagsSet=0;

      for (auto& f: classdesc::enum_keys<IncDecFlags>())
        {
          regex pat(f.second+".*:.*");
          if (regex_match(buf,pat))
            {
              c.incDec=f.first;
              flagsSet++;
            }
        }
      if (flagsSet>1)
        {
          cerr<<"only one of ";
          for (auto& f: classdesc::enum_keys<IncDecFlags>()) cerr<<f.second<<", ";
          cerr<<" allowed:\n";
          cerr<<buf<<endl;
        }
      
      c.shift=regex_match(buf,match,shift);
      c.mask0=regex_match(buf,match,mask);
      if (regex_match(buf,match,qualifiedOp) ||
          regex_match(buf,match,bareOp))
        {
          string op=match[1];
          for (auto& i: op) i=toupper(i);
          c.op=classdesc::enumKey<OpCodes>(op);
          int s=parseRegister(match[3]), d=parseRegister(match[5]);
          if (match[2]=="[")
            s+=numRegisters;
          if (match[4]=="[")
            d+=numRegisters;

          if (s>=16 || s>=numRegisters && match[2]!="[")
            {
              cerr<<"register "<<match[3]<<" invalid\n";
              cerr<<buf<<endl;
            }
          if (d>=16 || d>=numRegisters && match[4]!="[")
            {
              cerr<<"register "<<match[5]<<" invalid\n";
              cerr<<buf<<endl;
            }
  
          c.src=s;
          c.dst=d;
        }
      code.push_back(c.instr);
    }
}

string VectorCPU16bit::regName(int r)
{
  if (r>numRegisters)
    return "[R"+std::to_string(r-numRegisters)+"]";
  else if (r==numRegisters)
    return "[PC]";
  else if (r)
    return "R"+std::to_string(r);
  else
    return "PC";
}

namespace
{
  string tolower(string x)
  {
    for (auto& i:x) i=std::tolower(i);
    return x;
  }
}

void VectorCPU16bit::disassemble(ostream& text, const vector<Instr_set>& code)
{
  VectorInstr16bit c;
  for (auto i: code)
    {
      c.instr=i;
      if (c.incDec>0) text<<classdesc::enumKey<IncDecFlags>(c.incDec)<<" ";
      if (c.shift) text<<"shift ";
      if (c.mask0) text<<"mask0 ";
      text<<": ";
      // wrap around within normal and special op space
      if (c.op<numNormalops)
        text <<  tolower(classdesc::enumKey<OpCodes>
                         (c.op % numNormalops));
      else
        text <<  tolower(classdesc::enumKey<OpCodes>(c.op));

      text << " " << regName(c.src)<<","<<regName(c.dst)<<endl;
    }
}
