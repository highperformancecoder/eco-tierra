#ifndef MEMSZ
//#define MEMSZ 65536  /* must be larger than 2*(length of orgA + orgB) */
#define MEMSZ 2048 // for v4 comparison
#endif

#if (MEMSZ&(MEMSZ-1))
#error "MEMSZ must be a power of 2"
#endif

//#define soup_size 1048576
#define STACKSZ 16
/* this must be a power of 2 */
#if (STACKSZ&(STACKSZ-1))
#error "STACKSZ must be a power of 2"
#endif
#define MAXTEMPLSZ 10  /* if template larger than this, do an explicit
search through soup */


#ifndef SEARCHLIMIT
#define SEARCHLIMIT 1
#endif

#ifndef MAXSTEPS
#define MAXSTEPS 10000
#endif


#include <stdlib.h>
#include <string.h>
#include "oname.h"
#include <arrays.h>
#include <fcntl.h>
#include "result.h"
#include "deadMemInit.h"

/* instruction set */
enum instr_set {nop0,nop1,not0,shl,zero,ifz,subCAB,subAAC,incA,incB,decC,incC,
		pushA,pushB,pushC,pushD,popA,popB,popC,popD,jmpo,jmpb,
		call,ret,movDC,movBA,movii,adro,adrb,adrf,mal,divide,instr_sz};
/* instr_sz is one plus the highest instruction. Its numerical value
   gives the number of instructions in the instruction set */
typedef enum instr_set instr_set_t;  /* for dumb C++ compilers */

string instrName(instr_set);

enum finflag {notyet,reprod,oreprod};

extern enum instr_set soup[MEMSZ];

#ifndef CLASSDESC_ACCESS
#define CLASSDESC_ACCESS(x) 
#endif


class cpu
{
  CLASSDESC_ACCESS(cpu);
  enum instr_set genome[10000];
public:
  // where daughter cells are allocated wrt the parent. =
  // 2^Cell_bitsize when comparing with eco-tierra.4
  static const int daught_offs=512;

  int  start, size, inst_exec, startup, divs, allocatedSize; 
  int AX,BX,CX,DX,SP,PC, SPLowWater;
  int stack[STACKSZ]; 

#define templdatsz (2<<(MAXTEMPLSZ+1))
  struct templ_t {
    vector<vector<int> > data;
    int ptrs[templdatsz];
    /* with this data type, access template t of size s with index t|(2<<s) */
    vector<int>& operator[](int i) 
    {
      if (!ptrs[i])
	{
	  data.push_back(vector<int>());
	  ptrs[i]=data.size();
	}
      return data[ptrs[i]-1];
    }
    /*    templ_t() {for (int i=0; i<templdatsz; ptrs[i++]=0);}
    clear() called from init() anyway */
//    templ_t() {data.resize(templdatsz);}
    int count(int i) {if (ptrs[i]) return data[ptrs[i]-1].size(); else return 0;}
    void clear() {/*for (int i=0; i<templdatsz; ptrs[i++]=0);*/
      memset(ptrs,0,templdatsz*sizeof(ptrs[0])); data.clear();}
  } templ;
  int templsz;

  cpu *other;
  struct Saved {
    int AX,BX,CX,DX,SP,PC;
    int stack[STACKSZ]; 
    oname result;
  };
  Saved saved;
  bool div3sameState;
  enum finflag finished;
  float omatch, omatch_startup;
  oname  name, result, div1Result, div2Result;
  static DeadMemInit deadMemInit;
  cpu(): daughterAllocated(false) 
    {
      templsz=0;	
      memset(soup,0,MEMSZ*sizeof(soup[0]));
      other=NULL;
    }
  //  ~cpu() {}

  void updateTemplates(int start, int size);
  void init(char*, int, oname);
  /// trace print executations
  void prInstr();
  inline void execute();
  void push(int reg) {stack[SP++&(STACKSZ-1)]=reg;}
  void pop(int& reg) {
    reg=stack[--SP&(STACKSZ-1)];
    if (SP<SPLowWater) SPLowWater=SP;
  }
  void adr(int&, int&, int dir=0);

  void adjustPCafterAdr()
  {
    if (CX<8*sizeof(int))
      PC+=CX+1;
    else
      PC++;
  }

  // true if daughter cell is allocated (and writable)
  bool daughterAllocated;
  // number of instructions transferred to daughter cell
  unsigned long long movDaught;
  void Mal();
  void Divide();
  resultd_t results();
  int same_state();
  int interacts();
  enum instr_set& operator[](int x) {return soup[start+x];}
};

void tournament(cpu& orgA, cpu& orgB, resultd_t& f, resultd_t& r, int);

inline unsigned int abs(unsigned int x) {return x;}


inline void cpu::execute()
{
  int dummy, tmpPC;

  //prInstr();
  switch (soup[PC&(MEMSZ-1)])
    {
    case nop0: case nop1: PC++; break;
    case pushA: push(AX); PC++; break;
    case pushB: push(BX); PC++; break;
    case pushC: push(CX); PC++; break;
    case pushD: push(DX); PC++; break;
    case popA: pop(AX); PC++; break;
    case popB: pop(BX); PC++; break;
    case popC: pop(CX); PC++; break;
    case popD: pop(DX); PC++; break;

    case movDC: DX=CX; PC++; break;
    case movBA: BX=AX; PC++; break;
    case movii: 
      //  std::cout<<"movii ["<<AX<<"]="<<instrName(soup[BX])<<" succ="<<
      //  (AX<start+daught_offs+allocatedSize && AX>=start+daught_offs && 
      //   BX<MEMSZ && BX >=0 && daughterAllocated) << endl;
      if (AX<start+daught_offs+allocatedSize && AX>=start+daught_offs &&
          daughterAllocated) {
        soup[AX]=soup[BX&(MEMSZ-1)]; 
        movDaught++;
      }
      PC++; break;
    case subCAB: CX=AX-BX; PC++; break;
    case subAAC: AX=AX-CX; PC++; break;
    case incA: AX++; PC++; break;
    case incB: BX++; PC++; break;
    case incC: CX++; PC++; break;
    case decC: CX--; PC++; break;
    case zero: CX=0; PC++; break;
    case not0: CX |= !(CX&1); PC++; break;
    case shl: CX<<=1; PC++; break;
    case ifz: if (CX!=0) PC++; PC++; break;

      /* if PC is unchanged in this call (ie no match) then increment
         PC to prevent infinite recursion */
    case call: push(PC); 
    case jmpo: tmpPC=PC; adr(PC,dummy); 
      if (tmpPC==PC) 
        PC++; 
      break;
    case jmpb: tmpPC=PC; adr(PC,dummy,-1); if (tmpPC==PC) PC++; break;
    case ret: pop(PC); PC++; break;

    case adro: adr(AX,CX); adjustPCafterAdr(); break;
    case adrb: adr(AX,CX,-1); adjustPCafterAdr(); break;
    case adrf: adr(AX,CX,1); adjustPCafterAdr(); break;
    case mal:   Mal(); PC++; break;
    case divide: Divide(); PC++; break; 
    }

  while (PC>=MEMSZ) PC-=MEMSZ;
  while (PC<0) PC+=MEMSZ;
  
  inst_exec++;
}
