#if (defined(__GNUC__) && __GNUC__<=2)
#undef NO_HASH
#else
#define NO_HASH
#endif

#undef XDR_PACK /* xdr_pack doesn't work */

class pack_t;
typedef pack_t unpack_t;
#include <eco_string.h>

#if 0
/* add friend statements for each accessor function */
#define CLASSDESC_ACCESS(type)\
friend void pack(pack_t *,eco_string,type&);\
friend void unpack(unpack_t *,eco_string,type&);

//#ifdef __GNUC__
//#define CLASSDESC_ACCESS_TEMPLATE(type)\
//friend void pack<>(pack_t *,eco_string,type&);\
//friend void unpack<>(unpack_t *,eco_string,type&);
//#else
#define CLASSDESC_ACCESS_TEMPLATE(type) CLASSDESC_ACCESS(type)
//#endif
#endif


#include <pack_base.h>
#include <pack_stl.h>

/* a list of all the include files go here */
/* ANSI C API */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <signal.h>
#include <limits.h>
#include <float.h>
#include <time.h>

/* C++ */
#include "stl.h"
#include <iostream>
#include <strstream>

//using namespace std;
using std::vector;

/* Unix specific */
#include <unistd.h>
#include <sys/file.h>
#include <sys/times.h>

/* local modules */
#include "bigfloat.h"
#include "bigfloat.cd"
#include "result.h"
#include "oname.h"
#include "miniTierra.h"
#include <arrays.h>
//#include "classdesc_array.h"


//#ifndef MPI
//#define error error_nompi
//#endif
