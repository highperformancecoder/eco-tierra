#ifndef Soup_H
#define Soup_H

#include <ecolab.h>
#include "Wordsize.h"
#include "genebank.h"
#include <random.h>
#include <unordered_map>

using classdesc::ref;

template <class CPU>
struct Cell
{
  unsigned cellID, owner;
  typedef ::Rambank_entry<typename CPU::Instr_set> Rambank_entry;
  ref<Rambank_entry> organism;
  CPU cpu, cpuAtLastDiv;
  std::string firstResult, lastResult;
  /// if a daughter cell has been allocated by calling mal, but divide
  /// not yet called
  bool daughterAllocated; 
  bool inserted=false; ///< This cell was deliberately innoculated
  
  //typedef std::unordered_multimap<Word,Word> TemplateMap;
  typedef std::vector<std::vector<Word>> TemplateMap;
  
  TemplateMap templates;

  Result result;
  /// update result vector at division time in tournament mode
  void updateResult(const std::string& offspringName);

  Cell(unsigned id=0): cellID(id), owner(id), cpu(id), cpuAtLastDiv(id), daughterAllocated(false) {
    // for some completely unknown reason, this code crashes unless
    // organism is allocated
    *organism;
  }
  Cell(unsigned cellID, const ref<Rambank_entry>& organism): 
    cellID(cellID), owner(cellID), organism(organism), cpu(cellID),  cpuAtLastDiv(cellID), daughterAllocated(false)
  {updateTemplates();}

  size_t size() const {return organism? organism->genome.size(): 0;}

  void updateTemplates();

};

static const uWord mask=~0U<<Cell_bitsize;

template <class CPU>
class Soup
{
public:
  typedef ::Cell<CPU> Cell; 
  typedef typename CPU::Instr_set Instr_set;
  typedef ::Genebank<Instr_set> Genebank;
  typedef ::Rambank_entry<Instr_set> Rambank_entry;
  CLASSDESC_ACCESS(Soup);
private:
  Cell& get_cell_idx(uWord i) {
    uWord cell=(i&mask)>>Cell_bitsize;
    if (cell>=cells.size()) cell%=cells.size();
    return cells[cell];
  }
  size_t currentCell, maxCells;
  classdesc::Exclude<Genebank*> rambank;
  // used in tournament mode to ensure same allocations
  std::map<unsigned,unsigned> tournamentAllocations;

public:
  double slicePow;
  unsigned long long tstep;
  /// mutation parameters: instruction flaws, cosmic ray mutations and copyFlaws
  unsigned long long flawRate, mutRate, copyFlawRate;
  Word maxCellSz; ///< maximum cell size that can be allocated
  ecolab::urand uni; ///< uniform RNG for generating flaws and mutations 
  /// minimum number of writes to a daughter cell to enable division to go ahead
  double movPropThrDiv;

  /// in tournament mode, children are killed rather than
  /// started. Used for complexity analysis
  bool tournamentMode;

  ///< set the maximum no. cells in soup. pins data in  memory
  void setMaxCells(ecolab::TCL_args args) {
    maxCells = int(args); cells.reserve(maxCells);
    // it doesn't pay to cache more than number of cells
    rambank->reverseGenebank.max_elem = maxCells;
  }
  /// list of cells in the soup
  typedef std::vector<Cell> Cells;
  Cells cells;
  std::deque<unsigned> reaper_q;

  /// total soup size
  size_t memSz() const {return cells.size() * 1<<Cell_bitsize;} 

  void clear() {cells.clear(); currentCell=0; reaper_q.clear(); 
    tournamentAllocations.clear(); rambank->clear(); tstep=0;
  }

  /// returns instruction at location \a i
  int get(Word i) {
    uWord idx=i&~mask;
    Cell& cell = get_cell_idx(i);
    return (idx<cell.organism->genome.size())? cell.organism->genome[idx]: 0;
  }

  /// sets location \a i to \a val if cell is open for writing
  /// \a val is cast to/from instr_set
  bool set(Word i, int val) {
    uWord idx=i&~mask;
    Cell& cell = get_cell_idx(i);
    
    bool writable=cell.organism->name.size()==0 && 
      idx<cell.organism->genome.size();
    if (writable) cell.organism->genome[idx]=Instr_set(val);
    return writable;
  }
    
  void insert_genome(const ref<Rambank_entry>& organism);

  /// copy data from src to dest, with possible copy flaw
  bool movii(Word dest, Word src)
  {
//    std::cout << "movii ["<<dest<<"]="<<
//      classdesc::enum_keys<CPUInst0::instr_set>()(get(src))<<" succ="<<set(dest, get(src))<<std::endl;
    if (copyFlawRate && tstep % copyFlawRate == 0)
      {
        set(dest, uni.rand()*CPU::instr_sz);
        return false;
      }
    else
      return !set(dest, get(src));
  }

  /// allocate a cell from the reaper queue, returning address
  Word mal(Word size, unsigned owner);
  /// divide: make cell read only and start its CPU
  bool divide(Word cell);

  /// update the outgoing and incoming matches in the result structures
  void updateResultMatches(int src, int dest)
  {
    cells[src].result.outMatches++;
    cells[dest].result.inMatches++;
  }

  /// find template \a templ starting from \a PC. backwards if \a
  /// dir=-1, forwards if \a dir=1 and outwards if dir=0.
  Word adr(/*Word templ,*/ Word PC, Word& template_sz, int dir);
  /// find template \a templ starting from \a PC. backwards if \a
  /// dir=-1, forwards if \a dir=1 and outwards if dir=0.
  Word adrFromTempl(Word templ, Word PC, Word tEnd, int dir);
  Word adrFromTempl(Word templ, Word PC, int dir)
  {return adrFromTempl(templ,PC,PC+1,dir);}

  /// randomly replaces one memory location with random contents
  void mutate();

  void run(unsigned timeSlices);
  
  /// returns true if any cell interacts with any other. Useful for
  /// short circuiting tournaments
  bool interacts();

  Soup(Genebank& genebank): 
    slicePow(0), currentCell(0), rambank(&genebank),tstep(0), 
    flawRate(0), mutRate(0), copyFlawRate(0), maxCellSz(1<<Cell_bitsize), 
    tournamentMode(false), movPropThrDiv(0.7) {}

};

#include "Soup.cd"
#endif
