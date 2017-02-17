#include <ndbm.h>
#include <fcntl.h>
#include <stdio.h>

main(int argc, char *argv[])
{
  DBM *idb, *odb;
  int i;
  datum key, val;

  if (argc<3) 
    {
      printf("usage: %s <dbm files...> output\n",argv[0]);
      exit(0);
    }

  odb=dbm_open(argv[argc-1],O_WRONLY|O_CREAT,0644);

  for (i=1; i<argc-2; i++)
    {
      idb=dbm_open(argv[i],O_RDONLY,0);
      for (key=dbm_firstkey(idb); key.dptr!=NULL; key=dbm_nextkey(idb))
	{
	  val=dbm_fetch(idb,key);
	  if (val.dptr!=NULL)
	    dbm_store(odb,key,val,DBM_REPLACE);
	}
      dbm_close(idb);
    }
  dbm_close(odb);
}
