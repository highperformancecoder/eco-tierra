#include "neutrals.h"
#include <ecolab_epilogue.h>
using namespace std;

bool eq(const string& xname, const Result& x, 
       const string& yname, const Result& y)
{
  if (x.clas!=y.clas) return false;
  if (x.clas==Result::once || x.clas==Result::repeat )
    return (x.result==y.result || x.result==xname && y.result==yname)
      && x.firstDiv==y.firstDiv && x.copyTime==y.copyTime && 
      x.outMatches==y.outMatches && x.inMatches==y.inMatches;
  else return true; /*x.clas==infertile;*/
}

inline bool near(double x, double y)
{return fabs(x-y)<=.0001*fabs(x+y);}

bool near_eq(const string& xname, const Result& x, 
	    const string& yname, const Result& y)
{
  if (x.clas!=y.clas) return false;
  if (x.clas==Result::once || x.clas==Result::repeat )
    //    return x.firstDiv==y.firstDiv && x.tau==y.tau && x.mu==y.mu && x.nu==y.nu;
    return near(x.firstDiv,y.firstDiv) && near(x.copyTime,y.copyTime) && 
      near(x.outMatches,y.outMatches) && near(x.inMatches,y.inMatches) &&
      (x.result==y.result || x.result==xname && y.result==yname);
  else return true; /*x.clas==infertile;*/
}

// replace all instances of x with y in string s
void replaceAll(string& s, const string& x, const string& y)
{
  size_t p;
  while ((p=s.find(x))!=string::npos)
    s.replace(p,p+x.length(),y);
}

void extractSelfSelf(const string& selfName, const string& otherName, vector<string>& self, vector<string>& keys)
{
  for (vector<string>::iterator k=keys.begin(); k!=keys.end(); )
    if (k->find(selfName)!=string::npos || k->find(otherName)!=string::npos)
      {
        replaceAll(*k, selfName, "self");
        replaceAll(*k, otherName, "other");
        self.push_back(*k);
        keys.erase(k);
      }
    else
      ++k;
  sort(self.begin(), self.end());
}

/* return true if namei is neutrally equivalent to namej */
bool Neutrals::is_neutral(const string& i, const string& j)
{
  vector<string> resultKeysi=resultDbIdx[i], resultKeysj=resultDbIdx[j];

  // extract self-self interactions
  vector<string> selfi, selfj;
  extractSelfSelf(i, j, selfi, resultKeysi);
  extractSelfSelf(j, i, selfj, resultKeysj);

//  if (i=="0063aae" && j=="0060aac" || j=="0063aae" && i=="0060aac")
//    {
//      cout << i << ":" << j<<" keys: {";
//      for (size_t ii=0; ii<resultKeysi.size(); ++ii)
//        if (ii<resultKeysj.size() && resultKeysi[ii]!=resultKeysj[ii])
//          cout << "{"<<resultKeysi[ii]<<"}:{"<<resultKeysj[ii]<<"} ";
//      cout << "}\n";
//    }

  // can only be neutral if the resultKeys match
  if (resultKeysi!=resultKeysj) return false;

  // cout << i << " and " << j << " same1\n";
  if (selfi!=selfj) return false;
  
  // cout << i << " and " << j << " same2\n";

  // check the self-self interactions
  for (size_t k=0; k<selfi.size(); ++k)
    {
      // restore actual names
      replaceAll(selfi[k], "self", i);
      replaceAll(selfi[k], "other", j);
      replaceAll(selfj[k], "self", j);
      replaceAll(selfj[k], "other", i);
      if (!near_eq(i,resultDb[i+selfi[k]],j,resultDb[j+selfj[k]]))
        return false;
    }
     

  for (vector<string>::iterator k=resultKeysi.begin(); k!=resultKeysi.end(); ++k)
    if (!near_eq(i,resultDb[i+*k],j,resultDb[j+*k]))
      return false;
  return true;
}
    
void Neutrals::rem_neutrals()
{
  vector<bool> deleted(orgnms.size());
  for (vector<string>::iterator i=orgnms.begin()+1; i!=orgnms.end(); ++i)
    {
      phenotypes[*i]=*i; //is an archetypal genotype for this phenotype
      neutDb[*i].push_back(*i);
      for (vector<string>::iterator j=orgnms.begin(); j!=i; ++j)
        if (!deleted[j-orgnms.begin()] && is_neutral(*i,*j))
          {
            phenotypes[*i]=*j;
            neutDb[*j].push_back(*i);
            cout << *i <<" is a "<<*j<<endl;
            deleted[i-orgnms.begin()]=true;
            break;
          }
    }

  vector<string> uniqueOrgs;
  uniqueOrgs.reserve(orgnms.size());
  for (size_t i=0; i<orgnms.size(); ++i)
    if (!deleted[i])
      uniqueOrgs.push_back(orgnms[i]);
  orgnms.swap(uniqueOrgs);
}
