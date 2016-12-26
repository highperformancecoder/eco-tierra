#ifndef NEUTRAL_H
#define NEUTRAL_H

#include "result.h"

#include <vector>
#include <string>
#include <cachedDBM.h>

/// class supporting the classification of phenotypes
struct Neutrals
{
  std::vector<std::string> orgnms; ///< list of test organisms
  
  /// key is a space separated, lexicographically ordered list of
  /// organisms involved in the tournament, appended to the genome name
  /// of the organism occupying the cell for which the result is
  /// recorded.
  ecolab::cachedDBM<std::string, Result> resultDb;
  /// list of keys mentioning a given organism
  ecolab::cachedDBM<std::string, std::vector<std::string> > resultDbIdx;

  /// database of neutral equivalents
  ecolab::cachedDBM<std::string, std::vector<std::string> > neutDb;
  /// database of phenotypes
  ecolab::cachedDBM<std::string, std::string> phenotypes;

  /// calculate phenotypes of orgnms, and populate phenotypes and neutdb 
  void rem_neutrals();

  /// returns whether two organisms are neutrally equivalent
  bool is_neutral(const std::string& i, const std::string& j);

};

#include "neutrals.cd"
#endif

