#include "genebank.h"
#include <ecolab_epilogue.h>
#include <iostream>
using namespace std;
using namespace ecolab;
namespace cd=classdesc;

string tierraName(size_t size, size_t sequence)
{
  ostringstream o;
  o<<size;
  while (sequence>0)
    {
      o<<char('a'+sequence%26-1);
      sequence/=26;
    }
  return o.str();
}

bool Genebank::registerEntry(cd::ref<Rambank_entry>& entry)
{
  string name;
  Rambank::iterator existingEntry;

  // check rambank and genebank to see if code is there
  if (!reverseGenebank.key_exists(entry->genome)
      || (
          name=reverseGenebank[entry->genome], 
          existingEntry=rambank.find(cd::ref<Rambank_entry>(name)), 
          existingEntry == rambank.end()
          )
      && !key_exists(name))
    {
      cd::ref<Rambank_entry> entryCopy(*entry);
      if (reverseGenebank.key_exists(entry->genome))
        reverseGenebank.del(entry->genome); // delete offending key
      entryCopy->parent = entry->name;
      do // keep going until we get a new name
        entryCopy->name=
          tierraName(entry->genome.size(), ++lastId[entry->genome.size()]);
      while (rambank.count(entryCopy)||key_exists(entryCopy->name));
      insert(entryCopy);
      return true;
    }
  else
    {
      if (existingEntry == rambank.end())
        // entry exists only in genebank, so insert into rambank
        entry=insert((*this)[name]);
      else
        entry = *existingEntry;
      return false;
    }    
}

bool Genebank::findEntry(cd::ref<Rambank_entry>& entry)
{
  // check genebank to see if code is there
  return !reverseGenebank.key_exists(entry->genome)
    || (entry->name=reverseGenebank[entry->genome],
        !key_exists(entry->name));
}

string Genebank::archiveRambank()
{
  ecolab::eco_strstream extractedOrgs;
  vector<cd::ref<Rambank_entry> > toBeRemoved;
  
  for (Rambank::iterator i=rambank.begin();
       i!=rambank.end(); ++i)
    {
      if ((*i)->maxPop > savMinNum)
        {
          if (!key_exists((*i)->name))
            extractedOrgs << (*i)->name;
          (*this)[(*i)->name]=**i;
        }
      if ((*i)->population == 0)
        toBeRemoved.push_back(*i);
    }

  // remove extinct organisms
  for (size_t i=0; i<toBeRemoved.size(); ++i)
    rambank.erase(toBeRemoved[i]);
  return extractedOrgs.str();
}

cd::ref<Rambank_entry> Genebank::insert
(const cd::ref<Rambank_entry>& e)
{
  if (!e) return e;
  std::pair<Rambank::iterator, bool> res = rambank.insert(e);
  if (res.second) // entry was new, insert into reverse genebank
    reverseGenebank[e->genome]=e->name;
  return *res.first;
}

void  Genebank::populateReverseGenebank()
{
  if (opened())
    for (string name=firstkey(); !eof(); name=nextkey())
      reverseGenebank[(*this)[name].genome]=name;
}

string Genebank::rambankElem(TCL_args args)
{
  string name=args[-1].get<string>()+"_"+(char*)(args[0]);
  size_t i=(int)args;
  Rambank::iterator j=rambank.begin();
  for (; i && j!=rambank.end(); --i, ++j);
  if (j==rambank.end())
      throw error("rambank index %s out of range",name.c_str());
  TCL_obj(null_TCL_obj,name,*j);
  return name;
}


string Genebank::orgList()
{
  string orgList=firstkey();
  for (; !eof(); orgList+=" "+nextkey());
  return orgList;
}
