#ifndef ONAME_H
#define ONAME_H

#include <pack_stl.h>
#include <cachedDBM.h>
using namespace ecolab;
using namespace std;

/* label type for organisms */
const int oname_sz=7;

template<int s>
class oname_b
{
public:
  char data[s+1];
  oname_b() {data[s]='\0';}
  oname_b(const char *x) {strncpy(data,x,s); data[s]='\0';}
  oname_b(const oname_b& x) {strcpy(data,x.data);}
  oname_b& operator=(const oname_b& x){strncpy(data,x.data,s+1); return *this;}
  oname_b& operator=(const char *x){strncpy(data,x,s+1); return *this;}
  operator const char*() const {return data;}
  bool operator==(const oname_b& x) const {return strcmp(data,x.data)==0;}
  int operator!=(const oname_b& x){return !(*this==x);}
  /* use datum, not Datum to avoid spurious free of data */
  //  operator Datum() const {Datum r; r.copy_ptr(data,s+1); return r;}
};


//template<int s> void pack(pack_t*,eco_string,class oname_b<s>&);
//template<int s> void unpack(pack_t*,eco_string,class oname_b<s>&);

class oname: public oname_b<oname_sz>
{
  friend class poname;
public:
  oname():oname_b<oname_sz>() {}
  oname(const char *x): oname_b<oname_sz>(x) {}
  oname(const oname& x): oname_b<oname_sz>(x) {}
};

/* pair of onames used to index results database */
class poname: public oname_b<2*oname_sz>  
{
public:
  poname():oname_b<2*oname_sz>() {}
  poname(const char *x): oname_b<2*oname_sz>(x) {}
  poname(const oname& x): oname_b<2*oname_sz>(x.data) {}
  poname(const oname& x, const oname& y) 
  {strncpy(data,x.data,oname_sz); strncpy(data+oname_sz,y.data,oname_sz);}
};

namespace std 
{
  template <>
  class less<oname>
  {
  public:
    bool operator()(const oname& x, const oname& y)  const 
    {return strcmp((const char*)x,(const char*)y)<0;}
  };
 
  template <>
  class less<poname>
  {
  public:
    bool operator()(const poname& x, const poname& y)  const 
    {return strcmp((const char*)x,(const char*)y)<0;}
  };

  template <int s>
  istream& operator>>(istream& i, oname_b<s>& o)
  { 
    for (int j=0; j<s; ++j) i>>o.data[j];
    return i;
  }

  template <int s>
  ostream& operator<<(ostream& i, oname_b<s>& o)
  { 
    for (int j=0; j<s; ++j) i<<o.data[j];
    return i;
  }
}

typedef vector<oname> orglist;

/* this is for backward compatibility with existing databases */
inline pack_t& operator<<(pack_t& p, orglist& a)
{for (int i=0; i<a.size(); i++) p<<a[i]; return p;}

inline pack_t& operator>>(pack_t& p, orglist& a)
{
  oname t; 
  for (int i=0; i<p.size()/(oname_sz+1); i++) 
    {p>>t; a.push_back(t);}
  return p;
}

//#pragma omit TCL_obj oname
//#pragma omit TCL_obj poname
//#ifdef ECOLAB_LIB
//inline void TCL_obj(TCL_obj_t& targ, eco_string desc, oname& arg)
//{TCL_obj_register(desc,arg,NULL);}
//inline void TCL_obj(TCL_obj_t& targ, eco_string desc, poname& arg)
//{TCL_obj_register(desc,arg,NULL);}

//#endif
  
// specialised type to allow TCL access
#include <vector>
struct ovector: public std::vector<oname> {};

#include <iostream>
inline std::istream& operator>>(std::istream& i, ovector& v)
{ 
  v.clear();
  std::string o;
  while (i>>o) 
    v.push_back(oname(o.c_str()));
  return i;
}

inline std::ostream& operator<<(std::ostream& o, const ovector& v)
{
  for (ovector::const_iterator i=v.begin(); i!=v.end(); i++)
    o << (i!=v.begin()?" ":"") << *i;
  return o;
}

#include "oname.cd"
#endif
