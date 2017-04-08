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
        //        templates.insert(make_pair(t, (cellID<<Cell_bitsize) + i));
        if (t>=templates.size()) templates.resize(t+1);
        templates[t].push_back((cellID<<Cell_bitsize) + i);
      }
  
}

/**
   Find the closest memory location in range to \a PC, and return the value in match if found. Returns true if a valid template is found, false otherwise
*/
bool find_closest_match(Word PC, const Cell::TemplateMap& templates, Word t,
                        Word& match, int dir, Word soupSz)
{
  if (t>=templates.size()) return false;
  
  Word min_dist=std::numeric_limits<Word>::max();
  for (auto i: templates[t])
    {
      Word d=i-PC;
      if (d*dir>=0 && abs(d)<min_dist || 
          dir<=0 && abs(d-soupSz)<min_dist || dir>=0 && abs(d+soupSz)<min_dist) //wrap around
        {
          match=i;
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
bool find_closest_match_cell(Word PC, const Cell::TemplateMap& templates, Word t, Word& match, int dir, Word soupSz)
{
  if (t>=templates.size()) return false;

  Word min_dist=std::numeric_limits<Word>::max();
  for (auto i: templates[t])
    {
      Word d=i-PC;
      if (d*dir>=0 && abs(d)<min_dist) 
        {
          match=i;
          min_dist=abs(d);
        }
    }
  return min_dist!=std::numeric_limits<Word>::max();
}

// increment decrement with wraparound
void incDecCells(int& fcell, int& bcell, int max)
{
  if (++fcell>=max) fcell-=max;
  if (--bcell<0) bcell+=max;
}

Word Soup::adr(Word address, Word& size, int dir)
{
  Word i, t, g;
  /* load up template into t, */
  Cell& cell=get_cell_idx(address);
  auto& gen=cell.organism->genome;
  auto idx=address&~mask;
  for (i=idx+1, t=1; i<gen.size() && (g=gen[i])<=CPU::nop1; ++i)
    {
      t <<=1; 
      t |= !g;
    }
  size=i-idx-1;
  

  // just implement exact matching
  // now search outwards for matching templates
  int this_cell=cell.cellID;
  Word adr=(this_cell<<Cell_bitsize)|i&~mask; // place address in this_cell
  if (size==0 || size>=8*sizeof(Word)) return -1;

  // see if matching template is in current cell
  if (find_closest_match_cell(address,cells[this_cell].templates,t,
                              adr,dir, memSz())) 
    return adr;

  if (dir)
    for (int cell=this_cell+dir; ; cell+=dir)
      {
        //wrap around
        if (cell>=int(cells.size())) cell=0;
        if (cell==-1) cell=cells.size()-1;
        if (find_closest_match(address,cells[cell].templates,t,
                               adr,dir, memSz()))
          {
            updateResultMatches(this_cell, cell);
            return adr;
          }
        if (cell==this_cell) break;
     }
  else //check alternate directions if dir==0
    {
      int fcell=this_cell, bcell=this_cell;
      incDecCells(fcell,bcell,cells.size());
      for (; fcell!=this_cell && bcell!=this_cell; incDecCells(fcell,bcell,cells.size()))
        if (find_closest_match(address,cells[fcell].templates,t,adr,1, memSz()))
        {
          // check reverse direction in case closer match exists
          Word address_b=(this_cell<<Cell_bitsize)|i&~mask;
          if (find_closest_match(address,cells[bcell].templates,t,
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
          if (find_closest_match(address,cells[bcell].templates,t,adr,-1, memSz()))
            {
              updateResultMatches(this_cell, bcell);
              return adr;
            }
    }
  return adr;
}

void  Soup::insert_genome(const classdesc::ref<Rambank_entry>& organism)
{
  rambank->insert(organism);
  cells.push_back(Cell(cells.size(), organism));
  cells.back().cpu.active = true;
  cells.back().inserted=true;
  cells.back().organism->incrPopulation();
  // in tournamentMode, active cells are not killed.
  if (tournamentMode)
    {
      // allocate daughter cell next to parent for comparison with 3.x
      tournamentAllocations[cells.size()-1]=cells.size();
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
            {
              // try to find an empty cell
              for (auto& cell:cells)
                if (cell.organism->genome.empty())
                  {
                    cellID = tournamentAllocations[owner] = cell.cellID;
                    goto emptyCellFound;
                  }
              throw error("Please set maxCells parameter > %d",cells.size());
            emptyCellFound:;
            }
          else
            {
              cellID = tournamentAllocations[owner] = cells.size();
              cells.push_back(Cell(cells.size()));
            }
        }
      if (size!=cells[cellID].organism->genome.size())
        {
          cells[cellID].organism->genome.resize(size);
        }
      //      cells[cellID].updateTemplates();
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
      if (offspringName!="unknown" && cpu.divs>=1 && 
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
      //      cell.updateTemplates();
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
  double l2MemSz=log(cells.capacity() * 1<<Cell_bitsize)/log(2);
  int floorL2=l2MemSz;
  if (l2MemSz>floorL2) floorL2++;
  Word memMask=(1<<floorL2)-1;
  
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
          ++tstep;
          // perform cosmic ray mutations
          if (mutRate && tstep % mutRate == 0)
            mutate();
          // TODO: maybe cache instruction fetches
          // use the indexed version of cells in case cells changes
          CPU::Instr_set instr(CPU::Instr_set(get(cell.cpu.PC)));
          if (flawRate && tstep % flawRate == 0) // perform instruction flaws
            instr = CPU::Instr_set(uni.rand()* CPU::instr_sz);
          cell.cpu.execute(instr);
          cell.cpu.PC&=memMask;
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
    for (size_t j=0; j<cells[i].templates.size(); ++j)
      if (!cells[i].templates[j].empty()) 
        templates.insert(j);
   

  for (size_t i=0; i<cells.size(); ++i)
    {
      Cell& cell=cells[i];
      if (cell.templates.empty()) 
        cell.updateTemplates();
      // check for presence of the complement within cell in correct direction
      for (size_t i=0; i<cell.templates.size(); ++i)
        for (auto t: cell.templates[i])
        {
          // extract instruction prior to template
          Word instr=get(t-log2i(i)-1);
          if (!cell.cpu.adrMatchInstr(instr)) continue;
          Word ctmp=complement(i);

          if (ctmp<cell.templates.size())
            for (auto i: cell.templates[ctmp])
              if ((i-t)*cell.cpu.adrMatchDir(instr) >= 0)
                goto next_template; // match occurs in right direction within cell

          if (templates.count(ctmp))
            return true; // match in another cell
        next_template:;
        }
    }
  return false;
}
