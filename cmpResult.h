// allows one to compare eco-tierra.3 results with eco-tierra.4 results

#ifndef CMPRESULT_H
#define CMPRESULT_H

#include "result.h"

namespace eco_tierra_3
{

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
    //    operator ecolab::Datum() const {ecolab::Datum r; r.copy_ptr(data,s+1); return r;}
  };


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
    std::string first() const {return std::string(data, oname_sz);}
    std::string second() const {return std::string(data+oname_sz, oname_sz);}
    bool operator<(const poname& x) const {
      return std::string(data)<std::string(x.data);
    }
  };

  class Result
  {
  public:
    enum ::Result::Class  clas;
    oname result;
    float sigma,tau,mu,nu;
    Result() {sigma=tau=mu=nu=0; clas=::Result::noninteract; result="";}
    bool operator==(const ::Result& x) const
    {
      if (clas!=x.clas) return false;
      switch (clas)
        {
        case ::Result::noninteract: case ::Result::infertile:
          return true;
        case ::Result::once: case ::Result::nonrepeat:
          return (strlen(result.data)==0 && x.result=="unknown" ||
                  std::string(result.data)==x.result) ;
        case ::Result::repeat:
          return std::string(result.data)==x.result ;
                                                                                       }
      return false; // shouldn't be here
    }
    bool operator!=(const ::Result& x) const {return !operator==(x);}
  };

}

#include "cmpResult.cd"
#endif
