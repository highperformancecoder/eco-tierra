#include "etierraVCPU16.h"
#include "ecolab_epilogue.h"
#include <fstream>
#include <sstream>
using namespace std;

Etierra etierra;
make_model(etierra);
Soup<VectorCPU16bit>& soup=etierra.soup;

void Etierra::exportGenome(const std::string& filename, const std::string& genotype)
{
  ofstream f(filename);
  auto& rb=genebank[genotype];
  f<<"genotype: "<<rb.name<<endl;
  f<<"parent: "<<rb.parent<<endl<<endl;

  VectorCPU16bit::disassemble(f, rb.genome);
}
  
void Etierra::importGenome(const std::string& filename)
{
  string name, buf, parent, dummy;
  ifstream f(filename);
  string genoLabel="genotype:";
  string parentLabel="parent:";

  while (getline(f,buf) && !buf.empty())
    {
      if (buf.substr(0,genoLabel.length())==genoLabel)
        {
          istringstream is(buf);
          is >> dummy >> name;
        }
      if (buf.substr(0,parentLabel.length())==parentLabel)
        {
          istringstream is(buf);
          is >> dummy >> parent;
        }
    }

  cout << "importing "<<name<<endl;
  
  if (name.length())
    {
      auto& rb=genebank[name];
      VectorCPU16bit::assemble(rb.genome, f);
      rb.name=name;
      rb.parent=parent;
    }
}
