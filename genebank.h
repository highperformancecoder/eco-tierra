#ifndef RAMBANK_H
#define RAMBANK_H

#include "Wordsize.h"
#include "result.h"
#include <random.h>
#include <cachedDBM.h>
#include <pack_stl.h>

using classdesc::ref;

// size tracking vector (for performance)
class Genome: public std::vector<Instr_set>
{
  size_t m_size;
  CLASSDESC_ACCESS(Genome);
public:
  size_t size() const {return m_size;}
  void push_back(Instr_set x) {
    std::vector<Instr_set>::push_back(x); 
    m_size = std::vector<Instr_set>::size();
  }
  void pop_back() {
    std::vector<Instr_set>::pop_back(); 
    m_size = std::vector<Instr_set>::size();
  }
  void clear() {
    std::vector<Instr_set>::clear(); 
    m_size = std::vector<Instr_set>::size();
  }
  void resize(size_t s, Instr_set v=Instr_set()) {
    std::vector<Instr_set>::resize(s,v); 
    m_size = std::vector<Instr_set>::size();
  }

  iterator erase(iterator i) {
    iterator r=std::vector<Instr_set>::erase(i); 
    m_size = std::vector<Instr_set>::size();
    return r;
  }
  iterator erase(iterator b, iterator e) {
    iterator r=std::vector<Instr_set>::erase(b, e); 
    m_size = std::vector<Instr_set>::size();
    return r;
  }
  iterator insert(iterator i, Instr_set v=Instr_set()) {
    iterator r=std::vector<Instr_set>::insert(i, v); 
    m_size = std::vector<Instr_set>::size();
    return r;
  }
  template <class A1, class A2>
  iterator insert(iterator i, A1 a1, A2 a2) {
    iterator r=std::vector<Instr_set>::insert(i, a1, a2); 
    m_size = std::vector<Instr_set>::size();
    return r;
  }
};


/// entry into the Rambank. \c name is set at division
struct Rambank_entry
{
  std::string name, parent;
  //  Genome genome;
  std::vector<Instr_set> genome;
  unsigned nDivs;
  long long firstDiv, lastDiv; ///< track time of first & last division
  unsigned population, maxPop;
  Result result; ///< for complexity analysis 
  void incrPopulation() {
    ++population; if (maxPop<population) maxPop=population;
  }
  Rambank_entry(const std::string& name="", const std::string& parent=""): 
    name(name), parent(parent), nDivs(0), firstDiv(0), lastDiv(0),
    population(0), maxPop(0) {}
};

/// used to construct RamBank indexed by name
struct SortName
{
  bool operator()(const ref<Rambank_entry>& x, 
                  const ref<Rambank_entry>& y) const {
    return x->name<y->name;
  }
};

///// used to construct RamBank indexed by genome (reverse Rambank)
//struct SortGenome
//{
//  bool operator()(const classdesc::ref<Rambank_entry>& x, 
//                  const classdesc::ref<Rambank_entry>& y) const {
//    return x->genome<y->genome;
//  }
//};

//// this would not be necessary if cacheDBM allowed specification of comp as a template parameter
//namespace std
//{
//  template <> struct less<Rambank_entry>: public SortGenome {};
//}

class Genebank: public ecolab::cachedDBM<std::string,Rambank_entry>
{
  std::map<size_t,size_t> lastId;
  CLASSDESC_ACCESS(Genebank);
public:
  //typedef std::set<classdesc::ref<Rambank_entry>, SortName> Rambank;
  typedef std::set<ref<Rambank_entry>, SortName> Rambank;

  unsigned savMinNum; ///< threshold number for saving to genebank
  ecolab::cachedDBM<std::vector<Instr_set>, std::string> reverseGenebank;

private:
  Rambank rambank;
public:

  Genebank(): savMinNum(10) {}

  void clear() {
    commit(); 
    ecolab::cachedDBM<std::string,Rambank_entry>::clear();
    reverseGenebank.commit();
    reverseGenebank.clear();
    rambank.clear();
  }

  size_t rambankSize() const {return rambank.size();}
  ecolab::string rambankElem(ecolab::TCL_args args);

  /// populates reverseGenebank from the current genebank. Both DBMs need to be open. 
  void populateReverseGenebank();

  /// insert an entry into the rambank
  ref<Rambank_entry> insert(const ref<Rambank_entry>& e);
  
  /// looks up a Rambank entry corresponding to the genome given in
  /// entry. If it is a new organism, a new entry is created, and returns true.
  /// false otherwise.
  bool registerEntry(ref<Rambank_entry>& entry);
  /// looks up a Rambank entry corresponding to the genome given in
  /// entry, and updates the \a entry's name, but doesn't add the
  /// gen. If it is a new organism, true if returned.  false
  /// otherwise.
  bool findEntry(ref<Rambank_entry>& entry);

  /// archives all rambank entries exceeding genebanker thresholds and
  /// removes nonpresent entries
  //eco_strstream archiveRambank();
  std::string archiveRambank();

  /// returns list of organisms present in database
  std::string orgList();
};

// to allow the object browser to drill down into the Rambanker etc.
inline 
void TCL_obj(ecolab::TCL_obj_t& t, const classdesc::string& d, 
             const ref<Rambank_entry>& a)
{
  if (a) TCL_obj(t,d,const_cast<Rambank_entry&>(*a));
}

#include "genebank.cd"
#endif
