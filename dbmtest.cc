#include "dbm++.h"

main(int argc, char *argv[])
{
  cachedDBM<char*,char*> db("dbmtest");
  //  cachedDBM<int,char*> db("dbmtest");
    char *i;
    //int i;

  if (strcmp(argv[1],"write")==0)
    {
      
      db["Jan"]="Feb";
      db["Feb"]="Mar";
      db["Mar"]="Apr";
      /*
      db[1]="Feb";
      db[3]="Mar";
      db[5]="Apr";
      */
    }
  else if (strcmp(argv[1],"read")==0)
    {
      for (i=db.firstkey(); !db.eof(); i=db.nextkey())
      	cout << i << "=" << db[i] << endl;
            db["Mar"]="Jun";
      //db[5]="Jun";
    }
  else if (strcmp(argv[1],"delete")==0)
    {
            db.del("Feb");
      //      db.del(3);
       for (i=db.firstkey(); !db.eof(); i=db.nextkey())
      	cout << i << "=" << db[i] << endl;
    }
}
