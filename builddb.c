/*
  usage builddb dbmfile organisms
  */

#include <stdio.h>
#include <stdlib.h>
#include <ndbm.h>
#include <fcntl.h>

main(int argc, char* argv[])
{
  int i, size, j;
  char *org=NULL;
  unsigned buf;
  FILE *f;
  DBM *db;
  datum key, val;

  db = dbm_open(argv[1],O_RDWR|O_CREAT,0644);
  for (i=2; i<argc; i++)
    {
      sscanf(argv[i],"%4d",&size);
      free(org);
      org = (char*) malloc(size);
      f=fopen(argv[i],"r");
      for (j=0; !feof(f) && j<size; j++)
	{ 
	  fscanf(f,"%x",&buf);
	  org[j]=buf;
	}
      fclose(f);
      key.dsize=strlen(argv[i])+1;
      key.dptr=argv[i];
      val.dsize=size;
      val.dptr=org;
      dbm_store(db,key,val,DBM_INSERT);
      puts(argv[i]);
    }
  dbm_close(db);
}
      
