#ifndef ETIERRAVCPU16_H
#define ETIERRAVCPU16_H
#include "Soup.h"
#include "VectorCPU16bit.h"

class Etierra: public ecolab::TCL_obj_t
{
public:
  Soup<VectorCPU16bit> soup;
  Genebank<VectorCPU16bit::Instr_set> genebank;

  Etierra(): soup(genebank) {}
  
  void exportGenome(const std::string& filename, const std::string& genome);
  void importGenome(const std::string&);
  void injectOrg(const std::string& org) {
    if (!genebank.key_exists(org))
      throw ecolab::error("%s does not exist in the genebank", org.c_str());
    soup.insert_genome(genebank[org]);
  }
};

#include "etierraVCPU16.cd"
#endif
