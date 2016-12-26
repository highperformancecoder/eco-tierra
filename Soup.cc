#include "Soup.h"
#include "complement.h"
#include "error.h"
#include <ecolab_epilogue.h>
#include <limits>
#include <math.h>
#include <iostream>
#include <stdexcept>
#include <set>
using namespace std;
using ecolab::error;


void Cell::updateTemplates()
{
  templates.clear();
  for (size_t i=0; i<organism->genome.size(); ++i)
    if (organism->genome[i] <= CPU::nop1)
      {
        Word t(1);
        for (; i<organism->genome.size() && organism->genome[i] <= CPU::nop1; ++i)
          {
            t<<=1;
            t|=organism->genome[i];
          }
        templates.insert(make_pair(t, (cellID<<Cell_bitsize) + i));
      }
}

/**
   Find the closest memory location in range to \a PC, and return the value in match if found. Returns true if a valid template is found, false otherwise
*/
typedef pair<Cell::TemplateMap::const_iterator, 
             Cell::TemplateMap::const_iterator> Range; 
bool find_closest_match(Word PC, const Range& range, Word& match, int dir, Word soupSz)
{
  if (range.first==range.second) return false;
  
  Word min_dist=std::numeric_limits<Word>::max();
  for (Range::first_type i=range.first; i!=range.second; ++i)
    {
      Word d=i->second-PC;
      if (d*dir>=0 && abs(d)<min_dist || 
          dir<=0 && abs(d-soupSz)<min_dist || dir>=0 && abs(d+soupSz)<min_dist) //wrap around
        {
          match=i->second;
          if (d*dir>=0)
            min_dist=abs(d);
          if (dir<=0)
            min_dist=min(min_dist, abs(d-soupSz));
          if (dir>=0)
            min_dist=min(min_dist, abs(d+soupSz));
        }
    }
  return min_dist!=std::numeric_limits<Word>::max();
}

/**
   Find the closest memory location in range to \a PC, and return the value in match if found. Returns true if a valid template is found, false otherwise
*/
typedef pair<Cell::TemplateMap::const_iterator, 
             Cell::TemplateMap::const_iterator> Range; 
bool find_closest_match_cell(Word PC, const Range& range, Word& match, int dir, Word soupSz)
{
  if (range.first==range.second) return false;
  
  Word min_dist=std::numeric_limits<Word>::max();
  for (Range::first_type i=range.first; i!=range.second; ++i)
    {
      Word d=i->second-PC;
      if (d*dir>=0 && abs(d)<min_dist) 
        {
          match=i->second;
          min_dist=abs(d);
        }
    }
  return min_dist!=std::numeric_limits<Word>::max();
}

Word Soup::adr(Word address, Word& size, int dir)
{
  Word i,t, g;
  /* load up template into t, */
  Word cellLimit=(address&mask) + get_cell_idx(address).organism->genome.size();
  for (i=address+1, t=1; (g=get(i))<=CPU::nop1 && i<cellLimit; ++i)
    {
      t <<=1; 
      t |= !g;
    }
  size=i-address-1;
  

  // just implement exact matching
  // now search outwards for matching templates
  int this_cell=address>>Cell_bitsize;
  this_cell %= cells.size(); //wrap around
  Word adr=(this_cell<<Cell_bitsize)|i&~mask; // place address in this_cell
  if (size==0 || size>=8*sizeof(Word)) return -1;

  // see if matching template is in current cell
  if (find_closest_match_cell(address,cells[this_cell].templates.equal_range(t),
                         adr,dir, memSz())) 
    return adr;

  if (dir)
    for (int cell=this_cell+dir; ; cell+=dir)
      {
        //wrap around
        if (cell>=int(cells.size())) cell=0;
        if (cell==-1) cell=cells.size()-1;
        if (find_closest_match(address,cells[cell].templates.equal_range(t),
                               adr,dir, memSz()))
          {
            updateResultMatches(this_cell, cell);
            return adr;
          }
        if (cell==this_cell) break;
      }
  else //check alternate directions if dir==0
    for (int fcell=(this_cell+1)%cells.size(), bcell=(this_cell-1)%cells.size(); 
         fcell!=this_cell && bcell!=this_cell; 
         fcell++, bcell--,fcell%=cells.size(),bcell%=cells.size())
      if (find_closest_match(address,cells[fcell].templates.equal_range(t),
                              adr,1, memSz()))
        {
          // check reverse direction in case closer match exists
          Word address_b=(this_cell<<Cell_bitsize)|i&~mask;
          if (find_closest_match(address,cells[bcell].templates.equal_range(t),
                                 address_b,-1, memSz()))
            {
              Word db=bcell<this_cell? address-address_b: address-address_b+memSz();
              Word df=fcell>this_cell? adr-address: adr-address+memSz();
              if (db<df) 
                {
                  updateResultMatches(this_cell, bcell);
                  return address_b;
                }
            }
          updateResultMatches(this_cell, fcell);
          return adr;
        }
      else
        if (find_closest_match(address,cells[bcell].templates.equal_range(t),
                               adr,-1, memSz()))
          {
            updateResultMatches(this_cell, bcell);
            return adr;
          }

  return adr;
}

void  Soup::insert_genome(const classdesc::ref<Rambank_entry>& organism)
{
  rambank->insert(organism);
  cells.push_back(Cell(cells.size(), organism));
  cells.back().cpu.active = true;
  cells.back().organism->incrPopulation();
  // in tournamentMode, active cells are not killed.
  if (tournamentMode)
    {
      // allocate daughter cell next to parent for comparison with 3.x
      tournamentAllocations[cells.size()-1]=cells.size();
      cells.push_back(Cell(cells.size()));
    }
  else
    reaper_q.push_front(cells.size()-1);
  if (maxCells < cells.size()) maxCells = cells.size();
}


Word Soup::mal(Word size, unsigned owner)
{
  unsigned cellID;

  if (tournamentMode)
    {
      if (size < 0 || size > maxCellSz)
        size=0;
      // in tournament mode, we always allocate the same cell
      map<unsigned, unsigned>::iterator aCell =
        tournamentAllocations.find(owner);
      if (aCell != tournamentAllocations.end())
        cellID = aCell->second;
      else
        {
          if (cells.size()==cells.capacity())
            throw error("Please set maxCells parameter > %d",cells.size());
          cellID = tournamentAllocations[owner] = cells.size();
          cells.push_back(Cell(cells.size()));
        }
      if (size!=cells[cellID].organism->genome.size())
        {
          cells[cellID].organism->genome.resize(size);
        }
      cells[cellID].updateTemplates();
      cells[cellID].organism->name.clear();
      cells[cellID].owner=owner;
    }
  else  // regular mode
    {
      // throw exception if requested size exceeds maxCellSz
      if (size < 0 || size > maxCellSz)
        throw runtime_error("maxCellSz exceeded");

      if (cells.size() < maxCells)
        {
          // if we haven't filled our quota of cells, allocate some new ones
          cellID=cells.size();
          cells.push_back(Cell(cells.size()));
        }
      else
        {
      // otherwise kill the next organism to die, and reclaim its cell
          assert(tournamentMode || cells.size() == reaper_q.size());
          cellID=reaper_q.back();
          if (cells[cellID].cpu.active)
            cells[cellID].organism->population--;
          if (cells[cellID].owner != cellID)
            // we must also kill the owner of this cell to prevent it from
            // overwriting it
            {
              Cell& owner = cells[cells[cellID].owner];
              owner.cpu.active = false;
              owner.organism->population--;
            }
          reaper_q.pop_back();
        }
      Cell& alloced_cell=cells[cellID];
      // kill any previous org occupying cell
      alloced_cell.cpu.init(cellID);
      alloced_cell.organism.nullify();
      alloced_cell.organism->genome.resize(size);
      alloced_cell.organism->name.clear();
      alloced_cell.owner=owner;
      reaper_q.push_front(cellID);
    }
  cells[owner].daughterAllocated=true;
  return cellID << Cell_bitsize;
  
}

void Cell::updateResult(const string& offspringName)
{
  // update result structure
  if (result.clas == Result::noninteract || result.clas == Result::infertile)
    {
      result.clas = Result::once;
      result.result = offspringName;
      result.firstDiv=cpu.inst_exec;
    }
  else 
    {
      if (offspringName!="unknown" && cpu.divs>=2 && 
          cpu.sameState(cpuAtLastDiv) && lastResult==offspringName)
        {
          result.clas = Result::repeat;
          // with repeating replication record the repeated offspring
          result.result = offspringName;
        }
      else
        result.clas = Result::nonrepeat;
      // this ensures first viable replication is recorded for nonrepeat
      if (result.result.empty() || result.result=="unknown")
        result.result = offspringName;
      
      // note at this point, cpu.divs has not been updated with this
      // division, so cpu.divs refers to the number of divisions since
      // the first division
      result.copyTime = (cpu.inst_exec - result.firstDiv)/cpu.divs;
    }
  cpuAtLastDiv = cpu;
  lastResult=offspringName;
  cpu.stackLowWater=cpu.SP; // reset for sameState calc
}
      

bool Soup::divide(Word c)
{
  unsigned cellID = c >> Cell_bitsize;
  Cell& cell=cells[cellID];
  if (cell.cpu.active) return false; // already divided
  // threshold of daughter copy not exceeded
  Cell& parent=cells[cell.owner];
  if (!parent.daughterAllocated || parent.cpu.movDaught < movPropThrDiv*cell.size()) return false;
  parent.cpu.movDaught=0;
  parent.daughterAllocated=false;


  // register with the RamBank
  assert(cell.organism);
  cell.organism->parent=parent.organism->name;
  
  if (tournamentMode)
    {
      

      if (rambank->findEntry(cell.organism))
        {
          cell.organism->name = "unknown"; // don't actually create new names
          // for comparison with eco-tierra.3, actually check the
          // result against the active cells to see if cell.organism
          // is contained as a prefix
          for (Cells::iterator c=cells.begin(); c!=cells.end(); ++c)
            if (c->cpu.active && cell.size() >= c->size() &&
                memcmp(&c->organism->genome[0], &cell.organism->genome[0],
                       c->size()*sizeof(cell.organism->genome[0]))==0)
              {
                cell.organism->name=c->organism->name;
                break;
              }
        }
      assert(cell.organism);
      parent.updateResult(cell.organism->name);
      cell.updateTemplates();
    }
  else
    {
      rambank->registerEntry(cell.organism);
      assert(cell.organism);
      // update statistics
      if (cell.organism->firstDiv == 0)
        cell.organism->firstDiv = tstep;
      cell.organism->lastDiv = tstep;
      ++cell.organism->nDivs;
      cell.organism->incrPopulation();
  
      cell.owner=cellID; //now becomes self-owned
      cell.updateTemplates();
      cell.cpu.active = true; // start the CPU
    }
 return true;
}

void Soup::mutate()
{
  // compute total number of memory locations
  size_t nloc = 0;
  for (size_t i=0; i<cells.size(); ++i)
    nloc+=cells[i].size();

  double r=uni.rand() * nloc;
  for (size_t i=0, j=0; i<cells.size(); j+=cells[i].size(), ++i)
    if (r < j+cells[i].size())
      {
        // mutate within cell i. First create a copy of the cell
        // if we knew the ref was unique, we could omit the copy
        classdesc::ref<Rambank_entry> tmp; 
        tmp=(*cells[i].organism); //pin the old genome
        tmp->genome[r-j] = 
          CPU::Instr_set(uni.rand() * CPU::instr_sz);
        cells[i].organism = tmp;
        break;
      }
}

void Soup::run(unsigned timeSlices)
{
  // counter used to detect soup death
  int numInactiveCells=0;
  /* a timeslice is 10 * cell.size()^slicePow instructions */
  for (unsigned t=0; t<timeSlices; ++t, ++currentCell)
    {
      if (currentCell >= cells.size()) currentCell=0;
      Cell& cell=cells[currentCell];
      if (!cell.cpu.active) //move to next cell, do not advance timeslice
        {
          numInactiveCells++;
          if (numInactiveCells == cells.size())
            {
              cout <<"Soup died."<<endl;
              return;
            }
          --t;
          continue;
        }
      numInactiveCells=0;

      unsigned slice=10*pow(cell.organism->genome.size(), 
                            slicePow);
      for (unsigned i=0; i<slice; ++i)
        {
          Cell& cell=cells[currentCell];
          ++tstep;
          // perform cosmic ray mutations
          if (tstep % mutRate == 0)
            mutate();
          // TODO: maybe cache instruction fetches
          // use the indexed version of cells in case cells changes
          CPU::Instr_set instr(CPU::Instr_set(get(cell.cpu.PC)));
          if (tstep % flawRate == 0) // perform instruction flaws
            instr = CPU::Instr_set(uni.rand()* CPU::instr_sz);
          cell.cpu.execute(instr);
        }
    }
}

// log2 of an integer. Returns -1 if v=0
int log2i(unsigned v)
{
#ifdef __GNUC__
  return 8*sizeof(v)-1-__builtin_clz(v); // uses hardware support
#else
  const unsigned int b[] = {0x2, 0xC, 0xF0, 0xFF00, 0xFFFF0000};
  const unsigned int S[] = {1, 2, 4, 8, 16};
  int i;

  register unsigned int r = 0; // result of log2(v) will go here
  for (i = 4; i >= 0; i--) // unroll for speed...
    {
      if (v & b[i])
        {
          v >>= S[i];
          r |= S[i];
        } 
    }
#endif
}

bool Soup::interacts()
{
  // set of all templates in Soup
  std::set<Word> templates;
  for (size_t i=0; i<cells.size(); ++i)
    for (Cell::TemplateMap::const_iterator t=cells[i].templates.begin();
         t!=cells[i].templates.end(); ++t)
      templates.insert(t->first);
   

  for (size_t i=0; i<cells.size(); ++i)
    {
      Cell& cell=cells[i];
      if (cell.templates.empty()) 
        cell.updateTemplates();
      // check for presence of the complement within cell in correct direction
      for (Cell::TemplateMap::const_iterator t=cell.templates.begin();
           t!=cell.templates.end(); ++t)
        {
          // extract instruction prior to template
          Word instr=get(t->second-log2i(t->first)-1);
          if (!cell.cpu.adrMatchInstr(instr)) continue;
          Word ctmp=complement(t->first);
          auto range=cell.templates.equal_range(ctmp);
          for (auto i=range.first; i!=range.second; ++i)
            if ((i->second-t->second)*cell.cpu.adrMatchDir(instr) >= 0)
              goto next_template; // match occurs in right direction within cell

          if (templates.count(ctmp))
            return true; // match in another cell
        next_template:;
        }
    }
  return false;
}
