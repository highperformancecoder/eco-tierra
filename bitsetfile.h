//This definition allows the use of a 64 bit off_t for handling large files
#define _FILE_OFFSET_BITS=64
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>

//inline error system_error(char *memname)
//{
//  if (errno<sys_nerr)
//    return error("%s in %s()",sys_errlist[errno],memname);
//  else
//    return error("unknown error %d in %s",errno,memname);
//}

class bitsetfile
{
  char *data;
  bool writeable;
  int fd, pagesize, mapsize, setsize, setbytesize;
  off_t offs;

  /* round up i to nearest page boundary */
  off_t pageboundary(off_t i)  { return pagesize*(((i+pagesize-1)/pagesize));}
  void ensure_mapped(off_t i)
  {
    if (fd<0) throw error("file not open in ensure_mapped()");
    if (offs>=0 && (i+1)*setbytesize > offs+mapsize || i*setbytesize<offs )
      {
	if (munmap(data,mapsize)<0)
	  throw error("%s in ensure_mapped:munmap",strerror(errno));
	offs=-1;
      }
    if (offs<0) /* data not mapped yet */
      {
	offs=pageboundary(std::max(off_t(0), i*setbytesize - mapsize/2));
	data=(char*)mmap(0,mapsize,PROT_READ | (writeable? PROT_WRITE: 0),
		  MAP_SHARED, fd, offs);
	if (data==MAP_FAILED) 
	  throw error("%s in ensure_mapped:mmap",strerror(errno));
      }
  }

  bitsetfile(const bitsetfile&);
public:
  set_mapsize(int i) {mapsize=pageboundary(i);}
  Set_mapsize(TCL_args args) {set_mapsize((int)args);}
  bitsetfile(): fd(-1), offs(-1) {pagesize=getpagesize();}
  ~bitsetfile() {if (fd>=0) close(fd);} 
  close() {if (fd>=0) close(fd); offs=fd=-1;}
  init(char *fname, int ss, char *rw)
  {
    setsize=ss; setbytesize=(ss-1)/8+1;  //ss assumed >0
    writeable=rw[0]=='w';
    if (writeable)
      {
	fd=open(fname,O_RDWR|O_CREAT|O_TRUNC|O_LARGEFILE,0644);
	if (fd<0)  throw error("%s in init:open",strerror(errno));
	const int bufsize=4096;
	char buffer[bufsize]; memset(buffer,0,bufsize);
	const int nbufs=(double(ss)*setbytesize)/bufsize+1;
	for (int i=0; i<nbufs; i++)
	  if (write(fd,buffer,bufsize)<0) throw error("%s in init:write",strerror(errno));
      }
    else
      {
	fd=open(fname,O_RDONLY);
	if (fd<0)  throw error("%s in init:open",strerror(errno));
      }
  } 
  Init(TCL_args args) {init((char*)args,(int)args,(char*)args);}
  bool operator()(int i,int j)
  {
    ensure_mapped(i);
    return data[off_t(i)*setbytesize + (j>>3) - offs] & (1<<(j&7));
  }
  void set(int i,int j)
  {
    ensure_mapped(i);
    char *d=data+(off_t(i)*setbytesize + (j>>3) - offs);
    *d |= (1<<(j&&7));
  }
};

/* for checkpointing, bitsetfile is already backed to a disk file */
#pragma omit pack bitsetfile
#pragma omit unpack bitsetfile
void pack(classdesc::pack_t* t,const classdesc::string& d,bitsetfile& a){}
void unpack(classdesc::pack_t* t,const classdesc::string& d,bitsetfile& a){}
