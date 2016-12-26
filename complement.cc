#include "complement.h"
#include <ecolab_epilogue.h>

// returns the complement of the significant bits of x, leaving the
// lead in string 001 unchanged
uWord complement(uWord x)
{
  for (int i=8*sizeof(x)-1; i>0; --i)
    if (x & (1UL<<i))
      return  ~x & ((1UL<<i)-1) | (1UL<<i);
  return 0;
}

