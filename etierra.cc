#include "etierra.h"
#include <ecolab_epilogue.h>

#include <fstream>

etierra_t etierra;
make_model(etierra);

int eq(const oname& xname, const resultd_t& x, 
       const oname& yname, const resultd_t& y)
{
  if (x.clas!=y.clas) return 0;
  if (x.clas==once || x.clas==repeat )
    return (x.result==y.result || x.result==xname && y.result==yname)
      && x.sigma==y.sigma && x.tau==y.tau && x.mu==y.mu && x.nu==y.nu;
  else return 1; /*x.clas==infertile;*/
}

nbhd_ret etierra_t::test_mut(ecolab::array<int>& mut, int site, int m)
{
  
  int j;
  bool same=true, unviable=true;
  resultd_t fresult, rresult;
  for (j=0; j<orgnms.size() && (same ||unviable); j++)
    {
      tournament( orgnms[j], mut, fresult, rresult);
      same &= eq("mutant0",fresult,org,fpheno_base[j]) && 
	eq(orgnms[j],rresult,orgnms[j],rpheno_base[j]);
      unviable &= 
	(fresult.clas==infertile ||fresult.clas==noninteract||
	 fresult.result!=oname("mutant0")) && 
	(rresult.clas==infertile ||rresult.clas==noninteract||
	 rresult.result==oname("mutant0"));
    }
  return nbhd_ret(same,unviable,site,m);
}

inline void etierra_t::save_stats(const nbhd_ret& r)
{
  if (r.neut)
    {neut.sites[r.site]++; neut.muts[r.site]|=1<<r.mut;}
  if (r.unviable)
    {unviable.sites[r.site]++; unviable.muts[r.site]|=1<<r.mut;}
}

void etierra_t::handle_return()
{
#ifdef MPI_SUPPORT
  MPIbuf buf;
  nbhd_ret r;
  buf.get() >> r; 
  save_stats(r);
  idle.push_back(buf.proc);
#endif
}


void etierra_t::slave_loop()
{
#ifdef MPI_SUPPORT
  MPIbuf rbuf, sbuf;
  array<int> mut;
  int site, m;
  for (rbuf.get(); rbuf.size>0; rbuf.get())
    {
      rbuf >> mut >> site >> m;
      sbuf << test_mut(mut,site,m) << send(0);
    }
#endif
}
void etierra_t::Explore_nbhd(ecolab::array<int>& mut, unsigned limit)
{
  nbhd_ret r;
#if MPI_SUPPORT
  MPIbuf buf;
  if (idle.size()!=nprocs-1)
    { /* reinitialise idle vector */
      idle.clear();
      for (int i=1; i<nprocs; i++) idle.push_back(i);
    }
#endif
  for (int site=0; site<limit; site++)
    {
      for (int m=1; m<32; m++)
	{
	  mut[site]=m;
#if MPI_SUPPORT
	  if (nprocs>1 && (!idle.empty() || buf.msg_waiting()))
	    {
	      if (idle.empty()) handle_return();
	      buf << mut << site << m << send(idle.back());
	      idle.pop_back();
	    }
	  else
#endif
	    save_stats(test_mut(mut,site,m));
	}
      mut[site]=0;
    }
#if MPI_SUPPORT
  while (idle.size()<nprocs-1) handle_return();
  for (int i=1; i<nprocs; i++) buf.send(i);   /* terminate slaves */
#endif
}

      
void etierra_t::explore_nbhd(TCL_args args)
{
  parallel(args);
  if (myid>0) {slave_loop(); return;}

  ecolab::array<int> mut; istringstream ii((char*)args);
  ii>>mut;
  if (mut.size()!=torgs[org].size()) 
    error("mut and organism should be same length");
  unviable=neut=nbhd_stats(mut.size());
  int limit=args;

  Explore_nbhd(mut,limit);
}
  
void etierra_t::init_org(TCL_args args)
{
  org=(char*)args;
  fpheno_base.resize(orgnms.size());
  rpheno_base.resize(orgnms.size());
  ecolab::array<int> mut(torgs[org].size());
  mut=0;
  for (int j=0; j<orgnms.size(); j++)
    tournament( orgnms[j], mut, fpheno_base[j], rpheno_base[j]);
}

void etierra_t::init_orgnms(TCL_args args)
{
  char *orgs_fname=args;
  FILE* orgs=fopen(orgs_fname,"r");
  char buf[100];
  while (!feof(orgs)) 
    {
      fgets(buf,100,orgs);
      orgnms.push_back(oname(buf));
    }
}

void etierra_t::init_cpu(cpu& orgB, ecolab::array<int> mut)
{
  orgB.start=MEMSZ/2;
  //  char *seq=new char[mut.size];
  char seq[1000];
  Datum orgd=torgs[org];
  for (int i=0; i<mut.size(); i++) 
    seq[i]=(mut[i]+orgd.data()[i]) % instr_sz;
  orgB.init(seq, mut.size(),"mutant0");
  //  delete [] seq;
}

void etierra_t::tournament(oname opponent, ecolab::array<int>& mut, 
		resultd_t& forwardr, resultd_t& reverser)
{
  /* set up the two genotypes */
  
  cpu orgA, orgB;

  init_cpu(orgB,mut);
  
  if (opponent==oname("self000"))
    {
      while (orgB.inst_exec<500*orgB.size && orgB.divs<3) orgB.execute(); 
      forwardr=reverser=orgB.results();
    }
  else
    {
      orgA.start=0;
      orgA.init(torgs[opponent].data(),torgs[opponent].size(),opponent);
      ::tournament(orgA,orgB,forwardr,reverser, opponent==org);
    }
}


template<class key, class val>
void ecachedDBM<key,val>::Init(TCL_args args)
{
  char *fname=args, *mode=args;
  cachedDBM<key,val>::init(fname,mode[0]);
  if (!this->opened()) throw error("DBM file %s open failed",fname);
}
    
template<class key, class val>
void ecachedDBM<key,val>::close(TCL_args args)
{ cachedDBM<key,val>::close(); }

template<class key, class val>
void ecachedDBM<key,val>::set_max_elem(TCL_args args)
{ this->max_elem=args; }

oname onamedb::get(TCL_args args)
{ return (*this)[(char*)args]; }

void onamedb::set(TCL_args args)
{ 
  oname k=(char*)args, v=(char*)args;
  (*this)[k]=v;
}

void torg_header_t::load_header(TCL_args args)
{
  *(org_header*)this=(*this)[(char*)args];
}

void torg_header_t::store_header()
{
  (*this)[genotype]=*(org_header*)this;
}

void etierra_t::addorg(TCL_args args)
{
  eco_string dir=args; char* name=args;
  FILE *f=fopen((dir+"/"+name).c_str(),"r");
  if (!f) error("cannot open %s",name);
  char buf[100], code[1000000], *cptr=code; 
  int instr;
  do 
    {fgets(buf,100,f);} 
  while (strcmp(buf,"track 0:\n"));
  fgets(buf,100,f);
  while (fscanf(f,"%*s %*s %*d %x %*d",&instr)==1) *(cptr++)=instr;;
  torgs[name].packraw(code,cptr-code);
  fclose(f);
}

void etierra_t::addall(TCL_args args)
{
  std::string myname_add=std::string((char*)args[-1]);
  myname_add.erase(myname_add.begin()+myname_add.find_last_of('.'),
		   myname_add.end());
  myname_add+=".addorg";

  char *dir=args;
  tclcmd() << "foreach name [exec ls" << dir << "] {" <<
    myname_add.c_str() << dir << "$name}\n";
}

void etierra_t::addresults(TCL_args args)
{
  org=(char*)args;
  cpu orgA, orgB;
  resultd_t fresult, rresult;

  orgA.start=0; 
  orgB.start=MEMSZ/2; 

//  orgB.init(torgs[org].data(),torgs[org].size(),org);
//  while (orgB.inst_exec<10000*orgB.size && orgB.divs<3) orgB.execute(); 
//  fresult=orgB.results();
  orgA.init(torgs[org].data(),torgs[org].size(),org);
  while (orgA.inst_exec<10000*orgA.size && orgA.divs<3) orgA.execute(); 
  fresult=orgA.results();
  results[poname(org,oname("self000"))]=fresult;
  results[poname(oname("self000"),org)]=fresult;
  assert(fresult.clas!=noninteract);

  orgA.init(torgs[org].data(),torgs[org].size(),org);
  orgB.init(torgs[org].data(),torgs[org].size(),org);
  ::tournament(orgA,orgB,fresult,rresult,0);
  if (fresult.clas!=noninteract) 
    results[poname(org,org)]=fresult;

  for (int i=0; i<orgnms.size() && org!=orgnms[i]; i++)
    {
      cpu orgA, orgB;
      resultd_t fresult, rresult;

      orgA.other=&orgB;
      orgB.other=&orgA;

      orgA.start=0; 
      orgB.start=MEMSZ/2; 
      orgA.init(torgs[org].data(),torgs[org].size(),org);
      orgB.init(torgs[orgnms[i]].data(),torgs[orgnms[i]].size(),orgnms[i]);
      ::tournament(orgA,orgB,fresult,rresult,false);
      if (rresult.clas!=noninteract) 
          results[poname(org,orgnms[i])]=rresult;
      if (fresult.clas!=noninteract) 
	results[poname(orgnms[i],org)]=fresult;
      if (fresult.clas!=noninteract || rresult.clas!=noninteract)
        cout << (const char*)poname(orgnms[i],org) << " result:" << classn[fresult.clas] << " " << classn[rresult.clas] << std::endl;
    }
}

void etierra_t::create_key_exist()
{
  if (orgnms[0]!=oname("self000"))
    {
      orgnms.insert(orgnms.begin(),oname("self000"));
      create.insert(create.begin(),0);
      extinct.insert(extinct.begin(),0);
    }

  deleted =vector<bool>(orgnms.size(),false);
  std::map<oname,unsigned> pos;  
  for (unsigned i=0; i<orgnms.size(); i++)  pos[orgnms[i]]=i;
  cout << "key db made" << endl;
  fkey.resize(orgnms.size());
  rkey.resize(orgnms.size());
  int nkeys=0;
  for (poname key=results.firstkey(); !results.eof(); key=results.nextkey())
    {
      oname first(static_cast<const char*>(key)), 
	second(static_cast<const char*>(key)+oname_sz);
      if (pos.count(first) && pos.count(second))
	{
	  fkey[pos[first]].insert(pos[second]);
	  rkey[pos[second]].insert(pos[first]);
	  if ((++nkeys&0x2ff)==0)
	    cout << "\r" << nkeys << flush;
	}
    }
}

void etierra_t::display_sig(TCL_args args)
{
  oname org=(char*)args;
  clas_t c;
  if (results.opened()) 
    /* use precomputed database */
    {
      c=results[poname("self000",org)].clas;
      cout << org << ":self = " << classn[c] << endl;
      for (int i=0; i<orgnms.size(); i++)
	{
	  if ( (c=results[poname(orgnms[i],org)].clas)!=noninteract) 
	    cout << orgnms[i] <<":"<<org<<" = "<<classn[c]<<" result:"<<results[poname(orgnms[i],org)].result<<endl;
	  if ( (c=results[poname(org,orgnms[i])].clas)!=noninteract) 	    
	    cout << org<<":"<<orgnms[i] <<" = "<<classn[c]<<" result:"<<results[poname(org,orgnms[i])].result<<endl;
	}

    }
  else
    {
      cpu orgA, orgB;
      resultd_t fresult, rresult;
    
      orgA.start=0; 
      orgB.start=MEMSZ/2; 

//      orgB.init(torgs[org].data(),torgs[org].size(),org);
//      while (orgB.inst_exec<100000*orgB.size && orgB.divs<3) orgB.execute(); 
      orgA.init(torgs[org].data(),torgs[org].size(),org);
      //while (orgA.inst_exec<100000*orgA.size && orgA.divs<3) orgA.execute(); 
      cout << "at orgB.inst_exec="<<orgA.inst_exec<<endl;
      cout << org << ":self = " << classn[orgA.results().clas] << 
           " result: "<<orgA.results().result<<endl;
      for (int i=0; i<orgnms.size(); i++)
	{
          cpu orgA, orgB;
          resultd_t fresult, rresult;
          orgA.other=&orgB;
          orgB.other=&orgA;
      
          orgA.start=0; 
          orgB.start=MEMSZ/2; 
	  orgA.init(torgs[org].data(),torgs[org].size(),org);
	  orgB.init(torgs[orgnms[i]].data(),torgs[orgnms[i]].size(),orgnms[i]);
	  ::tournament(orgA,orgB,fresult,rresult,false);
	  if (rresult.clas!=noninteract)
	    cout << org << ":"<< orgnms[i] <<" = "<<classn[rresult.clas]<<" result:"<<rresult.result<<endl;
	  if (fresult.clas!=noninteract)
	    cout << orgnms[i] <<":"<<org<<" = "<<classn[fresult.clas]<<" result:"<<fresult.result<<endl;
	}
    }

}

inline int near(double x, double y)
{return fabs(x-y)<=.0001*fabs(x+y);}

bool near_eq(const oname& xname, const resultd_t& x, 
	    const oname& yname, const resultd_t& y)
{
  if (x.clas!=y.clas) return false;
  if (x.clas==once || x.clas==repeat )
    //    return x.sigma==y.sigma && x.tau==y.tau && x.mu==y.mu && x.nu==y.nu;
    return near(x.sigma,y.sigma) && near(x.tau,y.tau) && 
      near(x.mu,y.mu) && near(x.nu,y.nu) &&
      (x.result==y.result || x.result==xname && y.result==yname);
  else return true; /*x.clas==infertile;*/
}

/*return true if results[i,k]==results[j,k] and vice versa */
bool etierra_t::compare(const oname& i, const oname& j, const oname& k)
{
  if (k==i || k==j)
    return near_eq(i,results[poname(i,i)],j,results[poname(j,j)]);
  else if (k==oname("self000"))
    return near_eq(i,results[poname(i,k)],j,results[poname(j,k)]);
  else 
    return near_eq(k, results[poname(i,k)], k, results[poname(j,k)]);
}

bool etierra_t::rcompare(const oname& i, const oname& j, const oname& k)
{
  if (k==i || k==j) 
    return true;  //assumes compare called prior to rcompare
  else if (k==oname("self000"))
    return near_eq(i, results[poname(k,i)], j, results[poname(k,j)]);
  else 
    return near_eq(i, results[poname(k,i)], j, results[poname(k,j)]);
}

/* return true is namei is neutrally equivalent to namej */
bool etierra_t::is_neutral(const oname& i, const oname& j)
{
  for (int k=0; k<orgnms.size(); k++)
    if (!compare(i,j,orgnms[k]) || !rcompare(i,j,orgnms[k]))
      return false;
  return true;
}

/* return true is orgnms[i] is neutrally equivalent to orgnms[j] */
bool etierra_t::is_neutral(int i, int j)
{
  if (fkey[i]!=fkey[j] || rkey[i]!=rkey[j]) return false;
  for (set<unsigned>::iterator k=fkey[i].begin(); k!=fkey[i].end(); k++)
    if (!compare(orgnms[i],orgnms[j],orgnms[*k])) return false;
  for (set<unsigned>::iterator k=rkey[i].begin(); k!=rkey[i].end(); k++)
    if (!rcompare(orgnms[i],orgnms[j],orgnms[*k])) return false;
  return true;
}

bool etierra_t::Is_neutral(TCL_args args)
{
  oname namei=(char*)args, namej=(char*)args;
  return is_neutral(namei,namej);
}

/* insert removed org into neutral variant list */
void etierra_t::insert(oname src, oname dest)
{
  //  orglist::iterator i;
  int i,j;
  neutdb[dest].push_back(src);
  if (!neutdb[src].empty())
    {
      j=neutdb[dest].size();
      neutdb[dest].resize(neutdb[dest].size()+neutdb[src].size());
      for (i=0; i<neutdb[src].size(); i++, j++)
	neutdb[dest][j]=neutdb[src][i];
    }
  printf("neutdb[%s].size()=%d\n",(const char*)dest,(int)neutdb[dest].size());
  neutdb.del(src);
}

/* remove organism i from orgnms and create, and update extinction record j */
void etierra_t::remove(int i, int dest)
{
  cout << "removing " << (const char*)orgnms[i] << endl;
  assert(orgnms.size()==create.size());
  assert(create.size()>i);
  orgnms.erase(orgnms.begin()+i);
  create.erase(create.begin()+i);
  extinct.erase(extinct.begin()+i);
}

  /* remove nonliving orgs */
void etierra_t::rem_nonliving()
{
  for (int i=0; i<orgnms.size(); i++)
    {
      if (fkey[i].size()==0 && rkey.size()==0) deleted[i]=true;
      cout << orgnms[i] << endl;
    }
}

void etierra_t::rem_neutrals(TCL_args args)
{
  if (orgnms[0]!=oname("self000"))
    {
      orgnms.insert(orgnms.begin(),oname("self000"));
      create.insert(create.begin(),0);
      extinct.insert(extinct.begin(),0);
    }

  int block_sz=orgnms.size();
  if (args.count)
    args >> block_sz;

  cpu orgA, orgB;
  int i,j;
  results.clear();

  for (i=min_i+1; i<min(min_i+block_sz+1, int(orgnms.size())); i++)
    {
      for (j=min_j+1; j<min(min_j+block_sz+1, i); j++)
	{
	  if (deleted[j]) continue;
	  if (is_neutral(i,j))
	    {
	      if (create[i]>create[j])
		{
		  insert(orgnms[i],orgnms[j]);
		  //		  remove(i--,j);
		  deleted[i]=true;
		  if (extinct[j]<extinct[i])  extinct[j]=extinct[i];
		  break;
		}
	      else
		{
		  insert(orgnms[j],orgnms[i]);
		  //		  remove(j--,i);
		  //		  i--;
		  deleted[j]=true;
		  if (extinct[i]<extinct[j])  extinct[i]=extinct[j];
		}
	    }
	}
    }
}	

void etierra_t::cleanup_neutrals()
{
  vector <oname> norgnms;
  vector <int> ncreate, nextinct;
  for (int i=1; i<orgnms.size(); i++)
    if (!deleted[i]) 
      {
	norgnms.push_back(orgnms[i]);
	ncreate.push_back(create[i]);
	nextinct.push_back(extinct[i]);
      }
  orgnms.swap(norgnms);
  create.swap(ncreate);
  extinct.swap(nextinct);
}

void etierra_t::build_neutdb(TCL_args args)
{
  vector<oname> orglist;
  {
    using namespace ecolab;
    istringstream s((char*)args); s >> orglist;
  }

  int i,j;
  cout << "orglist.size()"<<orglist.size()<<endl;
  for (i=1; i<orglist.size(); i++)
    for (j=0; j<i; j++)
      if (is_neutral(orglist[i],orglist[j]))
	{
	  printf("%s is equivalent to %s\n",orglist[i].data,orglist[j].data);
	  neutdb[orglist[j]].push_back(orglist[i]);
	  orglist.erase(orglist.begin()+i);
	  i--; break;
	}
}

void etierra_t::write_orgnms(TCL_args args)
{
  FILE *f=fopen(args,"w");
  assert(orgnms.size()==create.size());
  for (int i=0; i<orgnms.size(); i++)
    fprintf(f,"%s %d\n",(const char*)orgnms[i],create[i]);
  fclose(f);
}

inline string cast(oname& x)
{return (const char*)x;}

void etierra_t::create_inv_neutdb()
{
  /* uses orgnms to index into neutdb */
  tclvar neut_class("neut_class");
  for (oname name=neutdb.firstkey(); !neutdb.eof(); name=neutdb.nextkey())
    {
      neut_class[cast(name)]=cast(name);
      for (int j=0; j<neutdb[name].size(); j++)
	neut_class[cast(neutdb[name][j])]=cast(name);
    }
}

/* return number of site different between two organisms */
int etierra_t::hop_count(TCL_args args)
{
  oname namei((char*)args), namej((char*)args);
  Datum icode=torgs[namei], jcode=torgs[namej];
  Datum maxcode, mincode;
  if (icode.size()>jcode.size())
    {maxcode=icode; mincode=jcode;}
  else
    {maxcode=jcode; mincode=icode;}

  int hop_count=maxcode.size();
  /* try to minimise the hopcount by sliding the offset */
  for (int offs=0; offs<=maxcode.size()-mincode.size(); offs++)
    {
      int hops=maxcode.size()-mincode.size();
      for (int i=0; i<mincode.size(); i++)
	hops+= maxcode.data()[offs+i] != mincode.data()[i];
      if (hops < hop_count) hop_count=hops;
    }
  return hop_count;
}

void TCL_obj(TCL_obj_t *t,const eco_string& d, std::_Bit_reference) {}

//double etierra_t::complexity()
//{
//  DiGraph foodweb(unsigned(orgnms.size())); 
//  for (int i=2; i<orgnms.size(); i++)
//    for (int j=1; j<i; j++)
//      {
//	if (results[poname(orgnms[i],orgnms[j])].clas!=noninteract)
//	  foodweb.push_back(Edge(i,j));
//	if (results[poname(orgnms[j],orgnms[i])].clas!=noninteract)
//	  foodweb.push_back(Edge(j,i));
//      }
//  return ::complexity(foodweb);
//}

void etierra_t::load_ecollog(TCL_args args)
{
  // create an inverse neutral database
  map<oname,oname> neut_class;
  for (oname name=neutdb.firstkey(); !neutdb.eof(); name=neutdb.nextkey())
    {
      cout << "processing "<<name<<endl;
      neut_class[name]=name;
      for (int j=0; j<neutdb[name].size(); j++)
	neut_class[neutdb[name][j]]=name;
    }

  // now read in the ecol.log file
  ifstream ecollog((char*)args);
  unsigned time, t2=0;
  oname org1, org2;
  unsigned len1,len2;
  string tag1, tag2;
  m_maxt=0;
  while (ecollog >> time >> len1 >> tag1 >> len2 >> tag2)
    {
      sprintf(org1.data,"%04u%3s",len1,tag1.c_str());
      sprintf(org2.data,"%04u%3s",len2,tag2.c_str());
      if (neut_class.count(org1) && neut_class.count(org2))
        {
          interaction_counts[time]
            [make_pair(neut_class[org1],neut_class[org2])]++;
          if (time>m_maxt) m_maxt=time;
        }
      if (time>t2 && time % 1000 == 0)
        {
          t2=time;
          cout<<t2<<endl;
        }
    }
}

// a const object, that when default initialised picks the next
// available identifier
class UniqueID
{
  unsigned id;
public:
  static unsigned next_id;
  UniqueID(): id(next_id++) {}
  template <class T>
  operator T() {return id;}
};

unsigned UniqueID::next_id=0;

void etierra_t::foodwebAt(TCL_args args)
{
  unsigned t=args, window=args, t0;
  if (t>=window) 
    t0=t-window+1;
  else
    t0=0;

  // accumulate interactions over window
  InteractionCount accum_interactions;
  InteractionCount::iterator i;
  for (unsigned ti=t0; ti<=t; ++ti)
    for (i=interaction_counts[ti].begin(); i!=interaction_counts[ti].end(); ++i)
      accum_interactions[i->first]+=i->second;

  //a map of onames to numerical id
  map<oname, UniqueID> nodeID;
  // reset numerical ids to start from 0
  UniqueID::next_id=0;

  // Now store in the foodweb
  foodweb.clear();
  for (i=accum_interactions.begin(); i!=accum_interactions.end(); ++i)
    {
      unsigned node1=nodeID[i->first.first], node2=nodeID[i->first.second];
      if (node1!=node2)
        foodweb.push_back
          (Edge(node1, node2, i->second));
    }
}

double etierra_t::complexity()
{
  return ::complexity(foodweb);
}

char classCode(clas_t c)
{
  switch (c)
    {
    case once: return 'o';
    case repeat: return 'r';
    case nonrepeat: return 'n';
    case infertile: return 'i';
    case noninteract: return ' ';
    }
  return '?';
}

void etierra_t::interactionMatrix(TCL_args args)
{
  ofstream f((char*)args);
  // write out header of org names
  for (int i=0; i<oname_sz; ++i)
    {
      for (int j=0; j<oname_sz+2; j++) f<<' ';
      for (int j=0; j<orgnms.size(); j++) f<<orgnms[j].data[i];
      f<<'\n';
    }
  f<<'\n';

  if (results.opened()) 
    /* use precomputed database */
    for (int j=0; j<orgnms.size(); j++)
      {
        oname& org(orgnms[j]);
        for (int i=0; i<oname_sz; i++) f<<org.data[i];
        f<<' ';        
        f<<classCode(results[poname("self000",org)].clas);
        
        for (int i=0; i<orgnms.size(); i++)
          f<<classCode(results[poname(orgnms[i],org)].clas);
        f<<'\n';
      }
}
