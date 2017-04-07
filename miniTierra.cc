#ifdef ECOLAB_LIB
#include <ecolab.h>
#endif
#include <iostream>
#include "miniTierra.h"
#include "miniTierra.cd"
#include "result.h"
#include <ecolab_epilogue.h>
#include <algorithm>

using namespace std;

enum instr_set soup[MEMSZ];

bool traceflag=false;
  
DeadMemInit cpu::deadMemInit=noChange;

string instrName(instr_set x)
{
  return classdesc::enum_keys<instr_set>()(x);
}

void cpu::prInstr()
{
  cout << inst_exec<<":PC="<<PC <<":"<<instrName(soup[PC]) << endl;
}

void cpu::updateTemplates(int start, int size)
{
  // remove any existing templates in [start, start+size)
  for (int i=0; i<templ.data.size(); ++i)
      {
        vector<int>& v=templ.data[i];
        v.erase
          (remove_if(v.begin(), v.end(), [start,size](int x)
                     {return x>=start && x<start+daught_offs;}), 
           v.end());
      }
  int j, t;
  for (int i=0, maxtemp=0; i<size; i++)
    if (soup[start+i]<=nop1)
      {
	maxtemp++;
	templsz=(templsz>maxtemp)? templsz: maxtemp;
	for (j=0, t=0; j<maxtemp && soup[start+i-j]<=nop1 && j<=i; j++)
	  {
	    t |=  soup[start+i-j] << j;
#ifndef EXACT_TEMPLATE
	    templ[t|(2<<j)].push_back(start+i+1);
#endif
	  }
#ifdef EXACT_TEMPLATE
	if ((i-j==-1 || soup[start+i-j]>nop1) && (i==size-1 || soup[start+i+1]>nop1))
	  templ[t|(1<<j)].push_back(start+i+1);
#endif
	if (soup[start+i+1]>nop1 || maxtemp>=MAXTEMPLSZ) maxtemp=0;
      }
}

void cpu::init(char* seq, int ssize, oname org)
{
  unsigned t, maxtemp;
  int i,j;

  AX=BX=CX=DX=SP=0;
  PC=start;
  div3sameState=false;
  startup=-1;
  inst_exec=0;
  omatch=0;
  finished=notyet;
  divs=0;
  name=org;
  allocatedSize=0;
  result=""; div1Result=""; div2Result="";
  for (i=0; i<STACKSZ; i++) stack[i]=0;


  size=ssize;
  for (i=0; i<size; i++)
    genome[i] = soup[start+i] = (enum instr_set) seq[i];
  for (i=start+size; i<start+2*size; i++) soup[i]=nop0;

  templsz=0;
  templ.clear();
  updateTemplates(start, size);
}


static int search_org(cpu *org, unsigned t, int sz, int dir, int PC)
{
  int a, ad;
  int j;
  if (org && sz <= org->templsz && /*PC>=org->start && PC<org->start+org->size &&*/ org->templ.count(t|(1<<sz))!=0)
    {
      for (j=0; j<org->templ.count(t|(1<<sz)); j++)
        if (( (ad=org->templ[t|(1<<sz)][j]) - PC) * dir >= 0)
          break;
      if (j<org->templ.count(t|(1<<sz)))
        {
          for (j++; j<org->templ.count(t|(1<<sz)); j++)
            {
              a=org->templ[t|(1<<sz)][j];
              if (abs(ad-PC)>abs(a-PC) && (a-PC) * dir >= 0) ad=a;
            }
          // only return if PC and ad are in current cell
          if (PC/cpu::daught_offs == ad/cpu::daught_offs)
            return ad;
        }
    }
  return -1;
}

static int search_org_other(cpu *org, unsigned t, int sz, int dir, int PC)
{
  int a, ad;
  int j;
  if (org && sz <= org->templsz && org->templ.count(t|(1<<sz))!=0)
    {
      ad=org->templ[t|(1<<sz)][0];
      int d = ad-PC, min_dist=std::numeric_limits<int>::max();
      if (d*dir >=0)
        min_dist=abs(d);
        
      if (dir<=0)
        min_dist=min(min_dist, abs(d-MEMSZ));
      if (dir>=0)
        min_dist=min(min_dist, abs(d+MEMSZ));
      
      for (j=1; j<org->templ.count(t|(1<<sz)); j++)
        {
          a=org->templ[t|(1<<sz)][j];
          d = a-PC;
          if (d*dir>=0 && abs(d)<min_dist || 
              dir<=0 && abs(d-MEMSZ)<min_dist || 
              dir>=0 && abs(d+MEMSZ)<min_dist) //wrap around
            {
              ad=a;
              if (d*dir>=0) 
                min_dist=abs(d);
              if (dir<=0)
                min_dist=min(min_dist, abs(d-MEMSZ));
              if (dir>=0)
                min_dist=min(min_dist, abs(d+MEMSZ));
            }
        }
      return ad;
    }
  return -1;
}

// return smallest distance d, taking into account wrap arounds in direction dir
int roundMin(int d, int dir)
{
  switch (dir)
    {
    case 0:
      return (2*abs(d)<MEMSZ)? abs(d): MEMSZ-abs(d);
    case 1:
      return d<0? MEMSZ+d: d;
    case -1:
      return d<0? -d: MEMSZ-d;
    }
}

void cpu::adr(int& address, int& sz, int dir)
{
  unsigned j,t;
  int a, ad=-1;
  /* load up template into t, size into j */
  for (j=PC+1, t=0; soup[j]<=nop1 &&
         // stop at genome boundary
         j!=start+size &&
         j!=start+daught_offs+allocatedSize &&
         (!other || j!=other->start+other->size &&
          j!= other->start+other->allocatedSize+daught_offs)
         ; j++)
    {
      t <<=1; 
      t |= !soup[j];
    }
  sz=j-PC-1;
  int adr=PC+sz+1; // default return value if no match  
  /* zero template, or template too large no match */
  if (sz==0||sz>=8*sizeof(int)) return;  
  assert(t<(1<<sz));

  if (PC>=start && (!other || start>other->start || PC<other->start))
    {
      if ((ad=search_org(this,t,sz,dir,PC))>=0) 
        adr=ad;
      else if (other&&(ad=search_org_other(other,t,sz,dir,PC))>=0) 
        {
          adr=ad;
          // check going the other way
          if ((ad=search_org_other(this,t,sz,dir,PC))>=0 &&
              roundMin(ad-PC, dir)<roundMin(adr-PC, dir))
            adr=ad;
          omatch+=(float) other->size/SEARCHLIMIT;
        }
      else if ((ad=search_org_other(this,t,sz,dir,PC))>=0)
        adr=ad; // wrap around search
    }
  else
     {
       if (other&& (ad=search_org(other,t,sz,dir,PC))>=0) 
         adr=ad;
       else if ((ad=search_org_other(this,t,sz,dir,PC))>=0) 
        {
          adr=ad;
          // check going the other way
          if (other&& (ad=search_org_other(other,t,sz,dir,PC))>=0 &&
              roundMin(ad-PC, dir)<roundMin(adr-PC,dir))
            {
              adr=ad;
              omatch+=(float) other->size/SEARCHLIMIT;
            }
        }
       else if (other && (ad=search_org_other(other,t,sz,dir,PC))>=0)
         {
           adr=ad;
           omatch+=(float) other->size/SEARCHLIMIT;
         }
     }
  
  address=adr;
}


void cpu::Mal()
{
  int i, toAlloc=CX;
  if (CX<0 || CX>daught_offs)
    toAlloc=0;
  AX=start+daught_offs;
  if (!daughterAllocated) movDaught=0;
  daughterAllocated=true;
  if (allocatedSize>toAlloc)
    // zero out deallocated region
    for (i=AX+toAlloc; i<MEMSZ && i<AX+allocatedSize; ++i)
      soup[i]=nop0;
  //updateTemplates(start+daught_offs, toAlloc);
//#ifdef EXACT_TEMPLATE
//  if (allocatedSize>toAlloc && soup[AX+toAlloc-1]<=nop1)
//    {
//      // load up last template, which gets overlooked in this circumstance
//      int t=0, j=0;
//      for (; j<toAlloc && soup[AX+toAlloc-1-j]<=nop1; ++j)
//        t|=soup[AX+toAlloc-1-j] << j;
//      if (j<=MAXTEMPLSZ)
//        templ[t|(1<<j)].push_back(AX+toAlloc);
//    }
//#endif
  allocatedSize=toAlloc;
  switch (deadMemInit)
    {
    case setNop0:
      for (i=0; i<toAlloc; i++) soup[AX+i]=nop0;
    case randomise:
      for (i=0; i<toAlloc; i++) soup[AX+i]=instr_set(rand()%instr_sz);
    }
}

void cpu::Divide ()
{
  int i,matched=0;

  const double movPropThrDiv=0.7; //Tierra default for this parameter
  if (!daughterAllocated || movDaught<movPropThrDiv*allocatedSize)
    // div failed
    return;

  movDaught=0;
  daughterAllocated=false;

  divs++;

  //updateTemplates(start+daught_offs, allocatedSize);

  /* compare daughter with parent or with other org */
  if (allocatedSize>=size && (matched=!memcmp(soup+start+daught_offs,genome,size*sizeof(instr_set)))) result=name;
  else if (other!=NULL && allocatedSize>=other->size)
    {
      if (matched=!memcmp(soup+start+daught_offs,other->genome,other->size*sizeof(instr_set)))
        result=other->name;
    }

  if (!matched) 
    {
#if 0   /* for the moment, this is not so important */
      torg_t::iterator org;
      Datum seq;
      for (org=torgs.begin(); org!=torgs.end(); org++)
	{
	  seq=org->second;
	  /*
	  for (i=0, matched=1; i<seq.dsize; i++) 
	    matched &= soup[start+size+i]==(enum instr_set)seq.dptr[i];
	    */
	  if (matched=!memcmp(soup+start+daught_offs,seq.data,seq.size))
	    {
	      result=org->first;
	      break;
	    }
	}
#endif
    }

  if (!matched) result="unknown";

  /* if this is the first divide, save inst_exec and omatch */
  switch (divs)
    {
    case 1:
      startup=inst_exec;
      omatch_startup=omatch;
      div1Result=result;
      result="";
      // fall through
    default:
      // save first successful reproduction
      if (divs>1 && div2Result[0]=='\0' && strcmp(result,"unknown")!=0)
        div2Result=result;
      div3sameState=same_state();
      saved.AX=AX; saved.BX=BX; saved.CX=CX; saved.DX=DX; 
      saved.SP=SP; saved.PC=PC;
      saved.result=result;
      for (i=0; i<STACKSZ; i++) saved.stack[i]=stack[i];
      break;
    }
  SPLowWater=SP;

}


/* compare the sate of the CPU with the state at first division */
int cpu::same_state()
{
  int k,r;
  r = AX==saved.AX && BX==saved.BX && CX==saved.CX && 
    DX==saved.DX && /*SP >= saved.SP &&*/ PC==saved.PC && result==saved.result;
  if (r)
    {
      int offs=saved.SP-SP;
      //if (SPLowWater+offs<0) SPLowWater=offs;
      for (k=SPLowWater; k<SP; k++) 
        if (stack[k&(STACKSZ-1)]!=saved.stack[(k+offs)&(STACKSZ-1)])
          return false;
    }
  return r;
}

const int jmp_instr = (1<<jmpo) | (1<<jmpb) | (1<<call) | 
  (1<<adro) | (1<<adrb) | (1<<adrf);

const int fwd_instr = (1<<adrf), bwd_instr=(1<<jmpb) | (1<<adrb);

/* Check whether organism actually interacts with other by checking templates*/
int cpu::interacts()
{
  int i,j,k,l,t, ct;
  for (i=0; other && i<templsz; i++)
    {
      for (j=0; j<1<<i; j++)
	{
	  int tmask=(2<<i)-1;
          for (k=0; k<templ.count((t=(j&tmask))|(2<<i)); ++k)
            {
              int from = templ[t|(2<<i)][k];
              int instr=1<<soup[from-i-2];
              if (instr & jmp_instr)
                {
                  //loop over complements existing in current CPU
                  for (l=0; l<templ.count((ct=(~j&tmask))|(2<<i)); ++l) 
                    {
                      int to = templ[ct|(2<<i)][l];
                      // if match is in correct direction
                      if ((to-from)*
                          (int((instr&fwd_instr)!=0) - int((instr&bwd_instr)!=0)) >= 0)
                        goto next_template;
                    }

                  // check whether complement exists in other
                  if (other->templ.count(ct|(2<<i)))
                    return 1;
                }
            next_template:;
            }
        }
    }
  return 0;
}

resultd_t cpu::results()
{
  resultd_t resultd;
  if (strlen(result)==0 && strlen(div1Result)==0 && strlen(div2Result)==0) 
    /* nothing happening*/
    {
      resultd.clas=infertile;
    }
  else if (strcmp(result,"unknown")!=0 && divs>=3 && div3sameState)
    {
      resultd.clas=repeat;
      resultd.result=result;
      resultd.sigma=1.0/startup;
      resultd.tau=2.0/(inst_exec-startup);
      resultd.mu=omatch_startup;
      resultd.nu=(omatch-omatch_startup)/2.0;
    }
  else
  /* for this purpose, consider nonrepeat and once as the same - ie in
     nonrepeat, the second time around matched something in the
     database, whereas in once it didn't, even if they may be neutrally
     equivalent */
    {
      switch (divs)
        {
        case 0: resultd.clas=noninteract; break;
        case 1: resultd.clas=once; break;
        default: resultd.clas=nonrepeat; break;
        }

      // save first successful known reproduction, or unknown otherwise
      if (strlen(div1Result)>0 && (strcmp(div1Result,"unknown")!=0 || strlen(div2Result)==0)) 
        resultd.result=div1Result;
      else if (strlen(div2Result)>0 && (strcmp(div2Result,"unknown")!=0 || strlen(result)==0)) 
        resultd.result=div2Result;
      else
        resultd.result=result;
      resultd.sigma=1.0/startup;
      resultd.tau=0;
      resultd.mu=omatch_startup;
      resultd.nu=0;
    }
  /*
    {
      resultd.clas=nonrepeat;
    }
    */

#if 0
  printf("%s%s ->%s %s=%s %g %g %g %g\n",
(char*)name,(char*)other->name,classn[resultd.clas],(char*)resultd.result,
(char*)lastresult, resultd.sigma,resultd.tau,resultd.mu,resultd.nu);
#endif

return resultd;
}

void tournament(cpu& orgA, cpu& orgB, 
		resultd_t& forwardr, resultd_t& reverser, int do_anyway)
{
  orgA.other=&orgB;
  orgB.other=&orgA;

  forwardr.clas = noninteract;
  reverser.clas = noninteract;
  bool doA=do_anyway || orgA.interacts(), doB=do_anyway || orgB.interacts();

  bool Ainteracts=orgA.interacts(), Binteracts=orgB.interacts();
  
  //  while (doA || doB)
  // comparison with v 4
  if (!doA && !doB) return;
  while (1+orgA.inst_exec+orgB.inst_exec<10000*(orgA.size+orgB.size) && (orgA.divs<3 || orgB.divs<3))
    {
      doA &= (orgA.inst_exec<10000*orgA.size && orgA.divs<3);
      doB &= (orgB.inst_exec<10000*orgB.size && orgB.divs<3);

      for (int i=0; /*doA &&*/ i<10; ++i)
        orgA.execute();
      for (int i=0; /*doB &&*/ i<10; ++i)
        orgB.execute();
    }

  if (orgA.inst_exec>0)
    {
      reverser=orgA.results();
      if (!do_anyway && (!Ainteracts || reverser.mu==0 /*&& reverser.nu==0*/))  
	reverser.clas = noninteract;
    }
 if (orgB.inst_exec>0)
   {
     forwardr=orgB.results();
     if (!do_anyway && (!Binteracts ||  forwardr.mu==0 /*&& forwardr.nu==0*/))  
	forwardr.clas = noninteract;
   }
}  



