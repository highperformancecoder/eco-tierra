#ifndef DBMPP_H
#define  DBMPP_H

/* the buggers have not used prototypes in ndbm.h -- grrr! */
typedef void DBM;
struct datum {char *dptr; size_t dsize;};

/* I hope these are right! - appears to be true on Irix, Linux, Tru64 and AIX */
#define DBM_INSERT	0
#define DBM_REPLACE	1

extern "C" {
DBM* dbm_open(const char *,int,mode_t);
void dbm_close(DBM*);
datum dbm_fetch(DBM*, datum);
int dbm_store(DBM *, datum, datum, int);
int dbm_delete(DBM *, datum);
datum dbm_firstkey(DBM *);
datum dbm_nextkey(DBM *);
}

/* end of include ndbm.h */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <pack_base.h>
#include <pack_stl.h>

#ifndef TRAD_STL
namespace std
{template <class K, class T> class hash_map: public map<K,T> {};}
#pragma omit pack std::hash_map
#pragma omit unpack std::hash_map
#pragma omit TCL_obj std::hash_map
#endif
using std::hash_map;
using std::vector;

class Datum: public xdr_pack
{
public:
  operator datum() {datum x; x.dptr=data; x.dsize=size; return x;}
  Datum(const datum& x): xdr_pack(x.dsize) {packraw(x.dptr,x.dsize);}
  Datum(): xdr_pack() {}
  Datum(const Datum& x) {packraw(x.data,x.size);}
  Datum& operator<<(Datum& x) {packraw(x.data,x.size); return *this;}
  Datum& operator>>(Datum& x) {x.packraw(data,size); return *this;}
  template <class T> 
    Datum& operator<<(T& x) {(*(xdr_pack*)this)<<x; return *this;}
  template <class T> 
    Datum& operator>>(T& x) {(*(xdr_pack*)this)>>x; return *this;}
  Datum& operator=(Datum& x) {reseti(); packraw(x.data,x.size); return *this;}
  template <class T> Datum& operator=(T& x){reseti()<<x; return *this;}
  template <class T> operator T() {T x;  reseto()>>x; return x;}
};

#pragma omit pack Datum
#pragma omit unpack Datum

inline void pack(pack_t *t, eco_string d, Datum& x)
{*t<<x.size; t->packraw(x.data,x.size);}

inline void unpack(pack_t *t, eco_string d, Datum& x)
{
  int size; *t>>size;
  x.packraw(t->data+t->pos, size);
  t->seeko(size);
}


/* sets the equality operator for strings used in hash_maps */
namespace std {
template <>
class equal_to<char*>
{public: int operator()(char *x,char *y){return strcmp(x,y)==0;}};
}

/* make the main class a base class in order to derive a special case for 
   strings */

template<class key, class val>
class cachedDBM_base : public hash_map<key,val>
{
  DBM *file;
  datum kk;
  int readonly;
protected:
  vector<char*> allocated;  /* list of strings used for (char *) case */
  virtual void asg(const key& k, val& v) {hash_map<key,val>::operator[](k)=v;}
public:
  int max_elem;   /* limit number of elements to this value */
  cachedDBM_base() {file=NULL; max_elem=INT_MAX;}
  void init(const char *fname, char mode='w')
  {
    file=dbm_open((char*)fname,(mode=='w')?(O_RDWR | O_CREAT): O_RDONLY,0644); 
    readonly=mode=='r';
  }
  //  void init(const char *fname, char *mode="w") {init(fname,mode[0]);}
  void close() {if (file!=NULL) {commit(); dbm_close(file); file=NULL;}}
  virtual ~cachedDBM_base() {close();}
  bool opened() {return file!=NULL;}
  val& operator[] (const key& k) 
  {
    if (this->size()>=max_elem) {commit();} /* do a simple purge of database */
    if (!count(k) && file!=NULL) 
      {
	Datum dk; 
	dk<<k;
	datum vv=dbm_fetch(file,(datum)dk);
	if (vv.dptr!=NULL) {val v; Datum(vv)>>v; asg(k,v);}
      }
    return hash_map<key,val>::operator[](k);
  }
  void commit()    /* write any changes out to the file, and clear cache */
  {
    if (file!=NULL && !readonly)
      {
	typename hash_map<key,val>::iterator i;
	Datum k, v; 
	for (i=this->begin(); i!=this->end(); i++) 
	  {
	    k=i->first; v=i->second;
	    if (v.data!=NULL) dbm_store(file,(datum)k,(datum)v,DBM_REPLACE);
	  }
      }
    this->clear();
    for (int i=0; i<allocated.size(); i++)
      delete [] allocated[i];
  }
  void del(key k)   
  {
    if (file!=NULL)
      {
	Datum dk; 
	dk=k; 
	erase(k); 
	dbm_delete(file,(datum)dk);
      }
  }
  key firstkey() 
  {
    if (file!=NULL)
      {
	key k; 
	kk=dbm_firstkey(file);
	if (kk.dptr!=NULL) {k=Datum(kk); return k;}
      }
  }
  key nextkey() 
  {
    if (file!=NULL)
      {
	key k; 
	kk=dbm_nextkey(file); 
	if (kk.dptr!=NULL) {k=Datum(kk); return k;}
      }
  }
  int eof() {return kk.dptr==NULL || file==NULL;}
};

/* if key is a string, redefine operator[] */

template<class val>
class cachedDBM1_base: public virtual cachedDBM_base<char *,val>
{
protected:
  cachedDBM1_base(){}
public:
  val& operator[] (char* kk)
  { 
    char *k=new char[strlen(kk)+1];
    strcpy(k,kk);
    this->allocated.push_back(k);
    return cachedDBM_base<char*,val>::operator[](k);
  }
};

/* if value type is a string, override the assignment to the hash map element*/

template<class key>
class cachedDBM2_base: public virtual cachedDBM_base<key,char *>
{
  typedef char *string;
protected:
  cachedDBM2_base(){}
  virtual void asg(key& k,string& vv)
  {
    char *v=new char[strlen(vv)+1];
    strcpy(v,vv);
    this->allocated.push_back(v); 
    hash_map<key,char*>::operator[](k)=v;
  }
};

template<class key, class val>
class cachedDBM: public  cachedDBM_base<key,val>
{
public:
  cachedDBM(){}
  cachedDBM(const char* f, char mode='w'){this->init(f,mode);}
};

template<class val>
class cachedDBM<char *,val>: public cachedDBM1_base<val>
{
public: 
  cachedDBM(){}
  cachedDBM(const char* f, char mode='w'){this->init(f,mode);}
};

template<class key>
class cachedDBM<key,char *>: public cachedDBM2_base<key>
{
public: 
  cachedDBM(){}
  cachedDBM(const char* f, char mode='w'){this->init(f,mode);}
};

template <>
class cachedDBM<char *,char *>: 
  public virtual cachedDBM_base<char*,char*>, 
  public cachedDBM1_base<char *>, 
  public cachedDBM2_base<char *>
{
public: 
  cachedDBM(){}
  cachedDBM(const char* f, char mode='w'){init(f,mode);}
};

#endif


