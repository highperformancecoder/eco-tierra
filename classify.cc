#include "eco-tierra.h"

torg_t torgs;
cachedDBM<poname,resultd_t> results;
vector<oname> orgnms;
vector<int> create;
int norgs, savei=0, savej=0;
char *pixmap;

#if 0
class  orglist: public vector<oname>
{
public:
  operator Datum() const
  {
    Datum r;
    for (int i=0; i<size(); i++) {r<<=operator[](i);}
    return r;
  }
  orglist& operator=(const Datum& x) 
  {for (int i=0; i<x.dsize; i+=oname_sz) {push_back(oname(x.dptr+i));}}
};
#endif

cachedDBM<oname,orglist> neutdb;
 
int fcmp(double x, double y)
{return (x<y)? -1: x>y;}

int rcmp(resultd_t x, resultd_t y)
{
  int r;
  if (x.clas!=y.clas) return x.clas<y.clas? -1:1;
  if (x.clas==infertile||x.clas==nonrepeat) return 0;
  if (r=strcmp(x.result,y.result)) return r;
  if (r=fcmp(x.sigma,y.sigma)) return r;
  if (r=fcmp(x.tau,y.tau)) return r;
  if (r=fcmp(x.mu,y.mu)) return r;
  if (r=fcmp(x.nu,y.nu)) return r;
  return 0;
}

int cmp(oname namei, oname namej)
{
  poname keyname;
  resultd_t resulti, resultj;
  int i, r;

  for (i=0; i<orgnms.size(); i++)
    {
      if (orgnms[i]==namei || orgnms[i]==namej)
	r=rcmp( results[poname(namei,namei)],
		results[poname(namej,namej)]);
      else
	r=rcmp( results[poname(namei,orgnms[i])],
		results[poname(namej,orgnms[i])] );
      if (r) return r==-1;
    }
  return 0;
}

void print(int i, oname org)
{
  resultd_t resultj, oresult;
  int j;
  float matchscale=6e-4, repscale=50;

  for (j=0; j<orgnms.size(); j++)
    {
      resultj=results[poname(orgnms[j],org)];
      oresult=results[poname(org,org)];

      /*      printf("%9s %7s %8.2f %8.2f %8.2f %8.2f ",classn[resultj.clas],
	  resultj.result,resultj.sigma,resultj.tau,resultj.mu,resultj.nu); */
      if (rcmp(oresult,resultj)==0 && org!=orgnms[j]) 
	resultj.clas=noninteract;/* no interaction */
      
      /* 
	 match parm goes to hue -> once to reddish, repeat to bluish
	 self-repros goes to full saturation, other goes to half
	 repro_rate -> brightness
	 */

#ifdef postscript
      switch (resultj.clas)
	{
	case infertile: printf(" 0 setgray "); break;
	case nonrepeat: printf(" .5 setgray "); break;
	case once: printf(" %g %g %g sethsbcolor ",
			  fmod(5/6.0+matchscale*resultj.mu,1), 
			  resultj.result==org? 1.0: .5, 
			  .5+repscale*resultj.sigma); break;
	case repeat: printf(" %g %g %g sethsbcolor ",
			    1/3.0+matchscale*resultj.nu, 
			    resultj.result==org? 1.0: .5, 
			    .5+repscale*resultj.tau); break;
	case noninteract: continue;
	}
      printf(" %d %d block\n",i,j);
#elif ppm
      char pixel[3], *p;
      switch (resultj.clas)
	{
	case infertile: pixel[0]=0; pixel[1]=0; 
	  if (org==orgnms[j])
	    pixel[2]=255;
	  else
	    pixel[2]=0; 
	  break;
	case nonrepeat: pixel[0]=127; pixel[1]=127; pixel[2]=127; break;
	case once: 
	  pixel[0]=255*fmod(5/6.0+matchscale*resultj.mu,1);
	  pixel[1]=resultj.result==org? 255: 127;
	  pixel[2]=255*(.5+repscale*resultj.sigma);
	  break;
	case repeat: 
	  pixel[0]=255*(1/3.0+matchscale*resultj.nu); 
	  pixel[1]=resultj.result==org? 255: 127; 
	  pixel[2]=255*(.5+repscale*resultj.tau); 
	  break;
	case noninteract: pixel[0]=255; pixel[1]=255; pixel[2]=255; break;
	}
      memcpy( pixmap+ 3*(i+orgnms.size()*j), pixel,3);
#endif
    }
#ifdef postscript
    printf("%d %d moveto (%s) Lshow\n",j+1,i,(const char*)org);
#endif
}

/* 
   argv[1]=tierra orgs file (with creation times)
   argv[2]=results file
   */

main(int argc, char* argv[])
{
  int i,j, restart_flag=0;
  FILE *dbi, *dbj, *orgs,*ckpt;
  char namei[100],namej[100];
  struct resultd *resulti;

  if (argc<3)
    {
      puts("Usage: classify createdb results");
      exit(0);
    }


  torgs.init(argv[1],'r');
  results.init(argv[2],'r'); results.max_elem=1000;

  /* load up organism list into memory */
  char name[8]; int creat;
  orgs=fopen(argv[1],"r");
  for (i=0; fscanf(orgs,"%7s%d",name,&creat)!=EOF; i++)
    {
      orgnms.push_back(name);
      create.push_back(creat);
    }
  
  sort(orgnms.begin(),orgnms.end(),cmp);

#if postscript
  puts("%!PS-Adobe-2.0 EPSF-2.0");
  printf("%%%%BoundingBox: 0 0 %d %d\n",3*orgnms.size()+20,3*orgnms.size()+3);
  puts("/Courier findfont 1 scalefont setfont");
  puts("/block {moveto 1 0 rlineto 0 1 rlineto -1 0 rlineto 0 -1 rlineto fill} def");
  puts("/Lshow {0 setgray show } def");
  puts("3 3 scale");
#elif ppm
  pixmap=new char[3*orgnms.size()*orgnms.size()];
  printf("P6 %d %d 255\n",orgnms.size(),orgnms.size());
#endif

  for (i=0; i<orgnms.size(); i++) print(i,orgnms[i]);

#if postscript
  puts("showpage");
#elif ppm
  fwrite(pixmap,1,3*orgnms.size()*orgnms.size(),stdout);
#endif
  for (i=0; i<orgnms.size(); i++) 
    cerr <<i << ' '<<orgnms[i]<<endl;
}











