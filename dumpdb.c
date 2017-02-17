#include <ndbm.h>
#include <fcntl.h>
#include <stdio.h>

main(int argc, char *argv[])
{
  DBM *idb;
  FILE *out;
  datum key, val;

  if (argc<3) 
    {
      printf("usage: %s db text\n",argv[0]);
      exit(0);
    }

  idb=dbm_open(argv[1],O_RDONLY,0);
  out=fopen(argv[2],"w");
  for (key=dbm_firstkey(idb); key.dptr!=NULL; key=dbm_nextkey(idb))
    {
      val=dbm_fetch(idb,key);
      if (val.dptr!=NULL)
	{
	  fprintf(out,"\n%d %d\n",key.dsize,val.dsize);
	  fwrite(key.dptr,key.dsize,1,out);
	  fwrite(val.dptr,val.dsize,1,out);
	}
    }
}
