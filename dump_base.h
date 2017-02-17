#ifndef DUMP_BASE_H
#define DUMP_BASE_H
#include "eco_string.h"
#include "eco_strstream.h"
#include <fstream>
#include <string>
#include <pack_base.h>

class dump_t: public ofstream 
{
public:
  dump_t(string x): ofstream(x.c_str()) {};
  dump_t(const char *x): ofstream(x) {};
};

template <class T>
void dump(dump_t *t, eco_string desc, T& arg)
{*t << desc << ":=" << arg << endl;}


template <class T>
void dump(dump_t *t, eco_string desc, is_objptr dum, T*& arg)
{
  if (arg) 
    dump(t,desc,*arg);
  else
    *t << desc << ":=NULL" << endl;
}

/* default action for pointers is to throw an error message */
template <class T>
void dump(dump_t *targ, eco_string desc, T*& arg)
{error("Dumping arbitrary pointer data not implemented");}

template<class C, class T>
void dump(dump_t *targ, eco_string desc, C& c, T arg) {} 

/* now define the array version  */
#ifndef IS_ARRAY
#define IS_ARRAY
class is_array {};
#endif

template <class T>
void dump(dump_t *targ, eco_string desc, is_array ia, T *arg,  int ncopies)
{for (int i=0; i<ncopies; i++) 
  {
    eco_strstream e; 
    e|desc|"["|i|"]";
    dump(targ,e,arg[i]);
  }
}

#ifndef IS_STATIC
#define IS_STATIC
class is_static {};
#endif

inline void dump(dump_t *targ, eco_string desc, is_static s)
{
#ifdef STATIC_DBG
  cerr << "Static member "<<desc<<" not dumped" << endl;
#endif
}
#ifdef ARRAYS_H
#pragma omit array
#pragma omit iarray
#pragma omit par_addr_int
#pragma omit par_addr_double
#pragma omit par_addr_iarray
#pragma omit par_addr_array

inline void dump(dump_t *targ, eco_string desc, array& arg)
{
  dump(targ,desc,arg.size);
  dump(targ,desc,is_array(),(double*)arg,arg.size);
}

inline void dump(dump_t *targ, eco_string desc, iarray& arg)
{
  dump(targ,desc,arg.size);
  dump(targ,desc,is_array(),(int*)arg,arg.size);
}
  
inline void dump(dump_t *targ, eco_string desc, par_addr_int& arg)
{int tmp=arg; dump(targ,desc,tmp);}

inline void dump(dump_t *targ, eco_string desc, par_addr_double& arg)
{double tmp=arg; dump(targ,desc,tmp);}

inline void dump(dump_t *targ, eco_string desc, par_addr_iarray& arg)
{iarray tmp=arg; dump(targ,desc,tmp);}

inline void dump(dump_t *targ, eco_string desc, par_addr_array& arg)
{array tmp=arg; dump(targ,desc,tmp);}


#endif

#pragma omit eco_string
#pragma omit TCL_obj_t
#pragma omit eco_strstream
#pragma omit xdr_pack


#endif
