#include "ref.h"
#include <ecolab.h>
#include "Soup.h"
#include "etierra.h"
#include <ecolab_epilogue.h>

#include <time.h>
#include <fstream>
#include <iomanip>

using namespace ecolab;
using namespace std;

Etierra etierra;
make_model(etierra);
Soup& soup=etierra.soup;

void Etierra::InjectOrg(const string& oname)
{
  if (!genebank.key_exists(oname))
    throw error("%s does not exist in the genebank", oname.c_str());
  ::ref<Rambank_entry> genome(genebank[oname]);
  soup.insert_genome(genome);
}

void Etierra::importGenome(TCL_args filename)
{
  ifstream gen((char*)filename);
  string buf, name, parent;
  // extract genome name
  while (getline(gen,buf))
    if (strncmp(buf.c_str(),"genotype:",9)==0)
      {
        istringstream is(buf);
        string dummy;
        is >> dummy >> name >> dummy >> dummy >> dummy >> dummy >> parent;
      }
    else if (buf == "CODE") 
      {
        for (int i=0; i<3 && gen; getline(gen,buf), ++i); // skip 3 more lines
        break;
      }
    else if (buf == "track 0:") // targ produced files miss the CODE line
      {
        getline(gen,buf);
        break;
      }

  if (!gen) throw error("failed to find oganism code");

  Rambank_entry org(name, parent);

  string opcode, semi;
  int watchbits;
  Instr_set instr;

  while (gen>>opcode>>semi>>watchbits>>hex>>(int&)instr)
    {
      getline(gen,buf);
      org.genome.push_back(instr);
    }

  cerr << "adding "<<name<<" of size "<<org.genome.size()<<endl;
  genebank[name] = org;
}

void Etierra::exportGenome(TCL_args args)
{
  string name((char*)args);
  ofstream gen((char*)args);
  Rambank_entry& org=genebank[name];
  gen << "\nformat: 3 bits 0 Exsh TCsh TPs MFs MTd MBh\n";
  gen << "genotype: "<<name<<" genetic: 0,"<<org.genome.size()<<
    " parent genotype: "<<org.parent<<"\n";
  gen << "1st_daughter:  flags: 0  inst: 0  mov_daught: 0          breed_true: 1\n";
  gen << "2nd_daughter:  flags: 0  inst: 0  mov_daught: 0          breed_true: 1\n";
  time_t tm=time(0);
  gen << "Origin: InstExe: "<<org.firstDiv<<",0  clock: 0  "<<asctime(localtime(&tm));
  gen << "MaxPropPop: 0  MaxPropInst: 0 mpp_time: 0,0 \n";
  gen << "ploidy: 1  track: 0\n";
  gen << "\nCODE\n\ntrack 0:\n\n";

  vector<Instr_set>::const_iterator i=org.genome.begin();
  for (; i != org.genome.end(); ++i)
    gen << mnemonic[*i]<<"\t; 000 "<<setw(2)<<setfill('0')<<hex<<*i<<"\n";
}


bool Etierra::anyCellDivsLT3() const
{
  for (Soup::Cells::const_iterator cell=soup.cells.begin(); 
       cell!=soup.cells.end(); ++cell)
    if (cell->cpu.active && cell->cpu.divs<3)
      return true;
  return false;
}

void Etierra::runJoust()
{
  soup.tstep=1;
  unsigned long long maxTime=0;
//  for (Soup::Cells::iterator cell=soup.cells.begin(); 
//       cell!=soup.cells.end(); ++cell)
//    if (cell->cpu.active)
//      while (cell->cpu.inst_exec < 10000*cell->size() && cell->cpu.divs<3)
//        cell->cpu.execute(CPU::Instr_set(soup.get(cell->cpu.PC)));
    for (Soup::Cells::const_iterator cell=soup.cells.begin(); 
           cell!=soup.cells.end(); ++cell)
      if (cell->cpu.active)
          maxTime+=10000*cell->size();
    while (soup.tstep < maxTime && anyCellDivsLT3())
      soup.run(10);
}


void Etierra::insertResults()
{
  for (vector<Cell>::iterator cell=soup.cells.begin(); 
       cell!=soup.cells.end(); ++cell)
    if (cell->cpu.active && cell->organism && !cell->organism->name.empty())
      {

        multiset<string> genNames; // genome names of all other cells in soup 
        for (vector<Cell>::iterator c=soup.cells.begin(); 
             c!=soup.cells.end(); ++c)
          if (c!=cell && c->organism && !c->organism->name.empty() && c->inserted)
            genNames.insert(c->organism->name);

        // fold into a string
        string suffix;
        for (multiset<string>::iterator i=genNames.begin(); i!=genNames.end(); ++i)
          suffix+=" "+*i;
        
        // if more than one active cell is in play, and no in/out
        // matches have occurred, rewrite the result as a noninteract
        if (genNames.size()>0 &&
            cell->result.inMatches+cell->result.outMatches==0)
          cell->result.clas=Result::noninteract;

        if (genNames.size()==0 && cell->result.clas==Result::noninteract)
            cell->result.clas=Result::infertile;

        cout << "recorded result="<<classn[cell->result.clas]
             <<" result="<<cell->result.result<<
          " name="<< cell->organism->name<<" suffix="<<suffix<<
          " firstdiv="<<cell->result.firstDiv<<" inMatches="<<cell->result.inMatches<<" outMatches="<<cell->result.outMatches<<endl;

        // TODO: filter out 3-wise and n-wise reactions that are just
        // subsets of pair-wise reactions
        if (cell->result.clas!=Result::noninteract)
          {
            resultDb[cell->organism->name+suffix]=cell->result;
            resultDbIdx[cell->organism->name].insert(suffix);
          }
      }
}

char classCode(Result::Class c)
{
  switch (c)
    {
    case Result::once: return 'o';
    case Result::repeat: return 'r';
    case Result::nonrepeat: return 'n';
    case Result::infertile: return 'i';
    case Result::noninteract: return ' ';
    }
  return '?'; //shouldn't be here
}

void Etierra::interactionMatrix(TCL_args args)
{
  ofstream f((char*)args);
  istringstream is((char*)args);
  vector<string> orgnms;
  string buf;
  while (is>>buf) orgnms.push_back(buf);

  const int oname_sz=7;
  // write out header of org names
  for (int i=0; i<oname_sz; ++i)
    {
      for (int j=0; j<oname_sz+2; j++) f<<' ';
      for (int j=0; j<orgnms.size(); j++) f<<orgnms[j][i];
      f<<'\n';
    }
  f<<'\n';

  if (resultDb.opened()) 
    /* use precomputed database */
    for (int j=0; j<orgnms.size(); j++)
      {
        string& org(orgnms[j]);
        for (int i=0; i<oname_sz; i++) f<<org[i];
        f<<' ';        
        f<<classCode(resultDb[org].clas);
        
        for (int i=0; i<orgnms.size(); i++)
          if (org < orgnms[i])
            f<<classCode(resultDb[org+" "+org+" "+orgnms[i]].clas);
          else 
            f<<classCode(resultDb[org+" "+orgnms[i]+" "+org].clas);           
        f<<'\n';
      }
}

void Etierra::cmpResults()
{
  int count=0;
  for (eco_tierra_3::poname k=oldResultDb.firstkey(); !oldResultDb.eof(); 
       k=oldResultDb.nextkey())
    {
      eco_tierra_3::Result oldResult=oldResultDb[k];
      string org1=k.first(), org2=k.second();
      string newKey;
      if (org1=="self000") 
        newKey=org2;
      else if (org2=="self000")
        newKey=org1;
      else
        newKey=org1+" "+org2;
      bool keyPresent=resultDb.key_exists(newKey);
      Result newResult=resultDb[newKey];
      // if no record in DB, treat it as noninteract
      if (!keyPresent)
        newResult.clas=Result::noninteract;
      // infertile only relevant for the self records
      if (oldResult.clas==Result::infertile && newKey.find(' ')!=string::npos)
        oldResult.clas=Result::noninteract;

      count++;
      
      // in eco-tierra3, result needed to match one of the parents,
      // otherwise it is infertile
      if (newKey.find(' ')==string::npos // self records
          && oldResult.clas==Result::infertile && newResult.result=="unknown")
            newResult.clas=Result::infertile;
     
      if (oldResult!=newResult)
        {
          cout <<"keyPresent="<<keyPresent<<endl;
          cout <<"keys:"<<string(k.data) << " |"<<newKey<<"|"<<endl;
          cout << "class:"<<classn[oldResult.clas] << " " <<classn[newResult.clas]<<endl;
          cout << "result:"<<string(oldResult.result) << " " <<newResult.result<<endl;
          cout << "firstDiv:"<<(1/oldResult.sigma) << " " <<newResult.firstDiv<<endl;
        }
    }
  cout << count << " records compared"<<endl;
}

template <class K, class V> vector<K> listKeys(const cachedDBM<K,V>& db)
{
  vector<K> r;
  for (K key=const_cast<cachedDBM<K,V>&>(db).firstkey(); 
       !const_cast<cachedDBM<K,V>&>(db).eof(); 
       key=const_cast<cachedDBM<K,V>&>(db).nextkey())
    r.push_back(key);
  return r;
}

vector<string> Etierra::listKeys(const string& dbName)
{
  if (dbName=="resultDb") return ::listKeys(resultDb);
  if (dbName=="resultDbIdx") return ::listKeys(resultDbIdx);
  if (dbName=="genebank") return ::listKeys(genebank);
  return vector<string>();
}

