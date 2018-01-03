#ifndef ETIERRA_H
#define ETIERRA_H
#include "ref.h"
#include "result.h"
#include "cmpResult.h"
#include <cachedDBM.h>
#include "Soup.h"
#include "neutrals.h"

class Etierra: public ecolab::TCL_obj_t
{
public:
  Soup<CPUInst0> soup;
  Genebank<CPUInst0::Instr_set> genebank;
  /// key is a space separated, lexicographically ordered list of
  /// organisms involved in the tournament, appended to the genome name
  /// of the organism occupying the cell for which the result is
  /// recorded.
  ecolab::cachedDBM<std::string, Result> resultDb;
  /// list of keys mentioning a given organism
  ecolab::cachedDBM<std::string, std::set<std::string> > resultDbIdx;
  ecolab::cachedDBM<eco_tierra_3::poname, eco_tierra_3::Result> oldResultDb;

  Etierra(): soup(genebank) {}

  /// inject an organism from the genebanker 
  void injectOrg(ecolab::TCL_args args) {InjectOrg(args);}
  void InjectOrg(const std::string& oname);

  /// add an empty cell ie a spacer
  void addEmptyCell() {soup.cells.emplace_back(soup.cells.size());}

  void exportGenome(ecolab::TCL_args);
  void importGenome(ecolab::TCL_args);

  void run(ecolab::TCL_args args) {soup.run(args);}

  void runJoust();
  bool anyCellDivsLT3() const;

  /// insert results from the current run into resultDb
  void insertResults();

  /// for diagnostic purposes, output where resultDb and oldResultDb differ
  void cmpResults();

  /// list keys in \a database (resultDb, resultDbIdx, genebank)
  std::vector<std::string> listKeys(const std::string& db);

  /// used for classifying organisms into phenotypes
  Neutrals neutrals;

  /**
     output an interaction matrix
     @param filename output file name
     @param orglist list of organisms to loop over
  */
  void interactionMatrix(ecolab::TCL_args args);
};

#include "etierra.cd"
#endif
