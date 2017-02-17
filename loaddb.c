#include <ndbm.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

main(int argc, char *argv[])
{
  DBM *odb;
  FILE *in;
  datum key, val;

  if (argc<3) 
    {
      printf("usage: %s text db\n",argv[0]);
      exit(0);
    }

  odb=dbm_open(argv[2],O_WRONLY|O_CREAT|O_TRUNC,0644);
  in=fopen(argv[1],"r");
  key.dptr=NULL; val.dptr=NULL;

  while (!feof(in))
    {
      fscanf(in,"%d %d\n",&key.dsize,&val.dsize);
      key.dptr=realloc(key.dptr,key.dsize);
      val.dptr=realloc(val.dptr,val.dsize);
      fread(key.dptr,key.dsize,1,in);
      fread(val.dptr,val.dsize,1,in);
      dbm_store(odb,key,val,DBM_REPLACE);
    }
  dbm_close(odb);
}
