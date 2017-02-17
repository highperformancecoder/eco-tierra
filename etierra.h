#define MEMSZ 2048  /* must be larger than 2*(length of orgA + orgB) */
//#define soup_size 1048576
#define MAXSTEPS 10000

#undef XDR_PACK   /* xdr_pack doesn't seem to work too well in this context */

//#define TCL_OBJ_STL_H   /* force disable TCL_obj_stl functionality */

#include <ecolab.h>
#include "miniTierra.h"
#include <arrays.h>
using ecolab::array;
#include <cachedDBM.h>
#include "result.h"
#include "oname.h"
#include <netcomplexity.h>

/* information contained in Tierran gene file */
struct org_header 
{
  oname genotype, parent;
  unsigned origin;
  double MaxPropPop, MaxPropInst, MaxPop;
  org_header(): MaxPop(0) {}
};

struct nbhd_ret
{ 
  bool neut, unviable; 
  int site, mut;
  nbhd_ret(bool n, bool u, int s, int m): 
    neut(n), unviable(u), site(s), mut(m) {}
  nbhd_ret() {}
};

struct nbhd_stats
{
  ecolab::array<int> sites, muts;
  nbhd_stats(int size=0): sites(size), muts(size) {sites=0; muts=0;}
};

/* we need to declare this here so as to export some methods to
   TCL. We can't do this directly in dbm++.h, as dbm++.h is a header
   only, and doesn't contain compiled code */
template<class key, class val>
struct ecachedDBM: public cachedDBM<key, val>
{
  void init(const char *fname, char mode='w') 
  {cachedDBM<key, val>::init(fname,mode);}   /* why do we need this? */
  void Init(TCL_args);
  void close(TCL_args);
  void set_max_elem(TCL_args);
};

#ifdef _CLASSDESC
#pragma omit pack ecachedDBM
#pragma omit unpack ecachedDBM
#pragma omit TCL_obj cachedDBM
#endif

namespace classdesc_access
{
  template<class key, class val>
  struct access_pack<ecachedDBM<key,val> >:
    public NullDescriptor<pack_t> {};

  template<class key, class val>
  struct access_unpack<ecachedDBM<key,val> >:
    public NullDescriptor<pack_t> {};
}

typedef ecachedDBM<oname, Datum> torg_t;
typedef ecachedDBM<poname,resultd_t> resultdb_t;
typedef ecachedDBM<oname,vector<oname> > neutdb_t;
typedef ecachedDBM<oname,unsigned> extinct_t;
class onamedb: public ecachedDBM<oname,oname>
{
public:
  oname get(TCL_args); /* TCL accessible operator[] */
  void set(TCL_args); /* TCL accessible operator[]= */
};

class torg_header_t: public org_header, public ecachedDBM<oname, org_header>
{
public:
  void load_header(TCL_args); /* load gene header */
  void store_header(); /* load gene header */
};

#include <iostream>
std::istream& operator>>(std::istream& s,oname& a)
{for (char *i=a.data; i<a+oname_sz; i++) s>>*i; return s;}

std::ostream& operator<<(std::ostream& s,const oname& a)
{s<<(const char*)a; return s;}

eco_strstream& operator|(eco_strstream& s,const std::vector<oname>&a)
{for (int i=0; i<a.size(); i++) s<<a[i]; return s;}

eco_strstream& operator|(eco_strstream& s,std::vector<int>&a)
{for (int i=0; i<a.size(); i++) s<<a[i]; return s;}

class ivector: public std::vector<int> {};

std::istream& operator>>(std::istream& s,ivector&a)
{
  a.resize(0); 
  int t; 
  while (s>>t)  a.push_back(t);
  return s;
}

inline std::ostream& operator<<(std::ostream& o, const ivector& v)
{
  for (ivector::const_iterator i=v.begin(); i!=v.end(); i++)
    o << (i!=v.begin()?" ":"") << *i;
  return o;
}
class etierra_t: public TCL_obj_t
{
  nbhd_ret test_mut(ecolab::array<int>& mut, int site, int m);
  inline void save_stats(const nbhd_ret& r);
  void Explore_nbhd(ecolab::array<int>& mut, unsigned limit);
  /* handle MPI master-slave code */
  void handle_return();
  void slave_loop();
  vector<int> idle;
  void init_cpu(cpu& orgB, ecolab::array<int> mut);
  void tournament(oname opponent, ecolab::array<int>& mut, 
		resultd_t& forwardr, resultd_t& reverser);

  CLASSDESC_ACCESS(etierra_t);
  unsigned m_maxt;
  
public:
  ovector orgnms;
  // distribute the orgnms vector across processors
  void broadcastOrgs(TCL_args args) {
#ifdef MPI_SUPPORT
    parallel(args);
    MPIbuf() << orgnms << bcast(0) >> orgnms;
#endif
  }
  ivector create, extinct;
  vector<bool> deleted;
  oname org;
  torg_t torgs;
  torg_header_t torg_headers;
  resultdb_t results;
  int min_i, min_j;

  /* record absense of keys in results database */
  vector< set<unsigned> > fkey, rkey;
  neutdb_t neutdb;
  vector<resultd_t> fpheno_base, rpheno_base;
  nbhd_stats neut, unviable;


  /// initialisation strategy for mal()
  DeadMemInit& deadMemInit;
  etierra_t(): deadMemInit(cpu::deadMemInit) {}

  int norgnms() {return orgnms.size();} // for TCL access
  
  void init_orgnms(TCL_args);
  void init_org(TCL_args);
  void explore_nbhd(TCL_args);

  /* for adding organism code to torgs database */
  void addorg(TCL_args);
  void addall(TCL_args);

  /* add a row of results for an organism */
  void addresults(TCL_args);

  /* create key existence tables for results database */
  void create_key_exist();

  void rem_nonliving();
  void rem_neutrals(TCL_args);
  void build_neutdb(TCL_args);
  /** used for comparing neutrality in rem-neutrals */
  bool compare(const oname& namei, const oname& namej,
	       const poname& r1, const poname& r2);
  /*return true if results[namei,namek]==results[namej,namek] and vice versa */
  bool compare(const oname& namei, const oname& namej, const oname& namek);
  bool rcompare(const oname& namei, const oname& namej, const oname& namek);
  /* return true is namei is neutrally equivalent to namej */
  bool is_neutral(const oname& namei, const oname& namej);
  /* return true is orgnms[i] is neutrally equivalent to orgnms[j] */
  bool is_neutral(int i, int j);
  bool Is_neutral(TCL_args);
  /* insert removed org into neutral variant list */
  void insert(oname src, oname dest);
  /* remove organism i from orgnms and create */
  void remove(int i, int);
  void write_orgnms(TCL_args);
  void cleanup_neutrals();

  void display_sig(TCL_args);
  // display the interaction matrix in the Tk Image passed in
  void interactionMatrix(TCL_args);
  void create_inv_neutdb();
  int hop_count(TCL_args);
  double complexity(); ///< complexity of foodweb described by orgnms

  // ecol.log processing stuff
  typedef map<pair<oname,oname>,unsigned> InteractionCount;
  ecachedDBM<unsigned,InteractionCount> interaction_counts;
  unsigned maxt() const {return m_maxt;}
  ConcreteGraph<DiGraph> foodweb;
  void load_ecollog(TCL_args); //load up the ecol.log file
  void foodwebAt(TCL_args); //compute foodweb at time t

  urand uni;
  void random_rewire() {::random_rewire(foodweb,uni);}

};

#include "etierra.cd"
#include "pack_stream.h"
#include "TCL_obj_templates.h"

using std::istringstream;
using std::string;
using std::cout;
using std::endl;
using std::pair;
