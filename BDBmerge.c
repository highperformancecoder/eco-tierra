#include <db.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
  DB *idb, *odb;
  int i, last;
  DBT key, val;
  DBC* cursor;

  memset(&key,0,sizeof(key));
  memset(&val,0,sizeof(val));

  if (argc<3) 
    {
      printf("usage: %s <dbm files...> output\n",argv[0]);
      exit(0);
    }

  db_create(&odb,0,0);
  odb->open(odb,NULL,argv[argc-1],NULL,DB_HASH,DB_CREATE,0644);

  for (i=1; i<argc-1; i++)
    {
      db_create(&idb,0,0); 
      idb->open(idb,NULL,argv[i],NULL,DB_HASH,DB_RDONLY,0);
      idb->cursor(idb,NULL,&cursor,0);
      for (last=cursor->c_get(cursor,&key,&val,DB_FIRST);
	   !last;
	   last=cursor->c_get(cursor,&key,&val,DB_NEXT))
	odb->put(odb,NULL,&key,&val,0);
      cursor->c_close(cursor);
      idb->close(idb,0);
    }
  odb->close(odb,0);
}
