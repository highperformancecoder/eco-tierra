#include "VectorCPU.h"
#include "Soup.h"
#include <ecolab_epilogue.h>
#include <map>
#include <regex>
#include <iomanip>
using std::vector;
using std::map;
using std::string;
using std::istream;
using std::ostream;
using std::regex;
using std::smatch;

OpTable copy={
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
};

OpTable cmpl={
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},
  {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0}
};

OpTable addv={
  {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
  {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0},
  {2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1},
  {3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2},
  {4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3},
  {5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4},
  {6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5},
  {7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6},
  {8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7},
  {9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8},
  {10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9},
  {11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10},
  {12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11},
  {13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12},
  {14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13},
  {15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14}
};

OpTable carry={
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1},
  {0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1},
  {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1},
  {0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1},
  {0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1},
  {0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1},
  {0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};  

enum class SpecialOp {mal=0x20,div=0x21,adrf=0x22};

OpTable* optable[] {&copy,&cmpl,&addv,&carry};
   
size_t numOps=sizeof(optable)/sizeof(OpTable);

// reference to the global soup
extern Soup& soup;

void VectorCPU::execute(instr_set instr)
{
  int32_t src=registers[instr&0xff], dest=registers[(instr>>9)&0xff];
  if (instr&(1<<8))
    src=soup.get(src); //indirect addressing
  if (instr&(1<<17))
    dest=soup.get(dest); //indirect addressing
  auto opCode=(instr>>18) & 0x3F;
  if (opCode & 0x20) // special op
    doSpecialOp(opCode,instr,src,dest);
  else
    {
      auto& op=*optable[ opCode % numOps ]; // TODO - optimise for power of 2 numOps
      int32_t mask=(instr>>24)&0xF;
      dest = (mask&1)*op[dest&0xFF][src&0xFF]
        | ((mask>>1)&1)*(op[dest&(0xFF<<4)][src&(0xFF<<4)]<<4)
    | ((mask>>2)&1)*(op[dest&(0xFF<<8)][src&(0xFF<<8)]<<8)
    | ((mask>>3)&1)*(op[dest&(0xFF<<12)][src&(0xFF<<12)]<<12);
    }
  dest<<4*((instr>>28)&3); //apply shift bits  
  if (instr&(1<<17))
    soup.set(registers[(instr>>9)&0xff],dest);
  else
    registers[(instr>>9)&0xff]=dest;
  // apply increment/decrement
  if ((instr>>30)&1) registers[(instr>>9)&0xff]++;
  if ((instr>>31)&1) registers[(instr)&0xff]--;
  // increment PC
  registers[0]++;
}


void VectorCPU::doSpecialOp(int32_t opCode,instr_set instr, int32_t src,int32_t& dest)
{
  switch (opCode)
    {
    case int(SpecialOp::mal):
      dest=soup.mal(src,cellID);
      return;
    case int(SpecialOp::div):
      //TODO
      return;
    case int(SpecialOp::adrf):
      dest=soup.adr(instr&0xFF,registers[0],1);
      return;
    default:
      return; //undefined - is a nop
    }
}

int parseRegister(const string& reg)
{
  if (reg=="PC")
    return 0;
  else if (reg.length()>1)
    return stoi(reg.substr(1));
}

void assemble(vector<VectorCPU::instr_set>& code, istream& text)
{
  string buf;
  regex comment("^[:space:]*;");
  regex incrDst("incrDst.*:"), decrSrc("decrSrc.*:");
  regex shift("shift([0-9]).*:"), mask("mask([0-9,a-f,A-F]).*:");
  regex qualifiedOp(R"(:[:space:]*([:alnum:]*)[:space:]+(\[?)([:alnum:]*)\]?,(\[?)([:alnum:]*))");
  regex bareOp(R"(^[:space:]*([:alnum:]*)[:space:]+(\[?)([:alnum:]*)\]?,(\[?)([:alnum:]*)");
  smatch match;
  while (getline(text,buf))
    {
      if (regex_match(buf,comment)) continue;
      VectorInstr c;
      c.instr=0;
      c.incrDst=regex_match(buf,incrDst);
      c.decrSrc=regex_match(buf,decrSrc);
      if (regex_match(buf,match,shift))
        c.shift=stoi(match[1]) & 3;
      if (regex_match(buf,match,mask))
        c.mask=stoi(match[1],nullptr,16) & 0xF;
      else
        c.mask=0xF;
      if (regex_match(buf,match,qualifiedOp) ||
          regex_match(buf,match,bareOp))
        {
          string op=match[1];
          c.op=classdesc::enumKey<VectorCPU::OpCodes>(op);
          c.srcInd=match[2]=="[";
          c.src=parseRegister(match[3]);
          c.dstInd=match[4]=="[";
          c.dst=parseRegister(match[5]);
        }
      code.push_back(c.instr);
    }
}

void disassemble(ostream& text, const vector<VectorCPU::instr_set>& code)
{
  VectorInstr c;
  for (auto i: code)
    {
      c.instr=i;
      if (c.decrSrc) text<<"decrSrc ";
      if (c.incrDst) text<<"incrDst ";
      if (c.mask) text<<"mask"<<std::hex<<std::setw(1)<<c.mask<<" ";
      text<<": ";
      if (!c.notNop)
        text << "nop ";
      else
        // wrap around within normal and special op space
        if (c.op & 0x20)
          text <<  classdesc::enumKey<VectorCPU::OpCodes>
            (0x20 + (c.op & 0x1F) % VectorCPU::numSpecialOps)<<" ";
        else
          text <<  classdesc::enumKey<VectorCPU::OpCodes>
            (c.op % VectorCPU::numNormalops)<<" ";

      if (c.srcInd)
        text<<"[";
      if (c.src)
        text<<"R"<<std::setw(1)<<c.src;
      else
        text<<"PC";
      if (c.srcInd)
        text<<"],";

      if (c.dstInd)
        text<<"[";
      if (c.dst)
        text<<"R"<<std::setw(1)<<c.dst;
      else
        text<<"PC";
      if (c.dstInd)
        text<<"]\n";
    }
}
