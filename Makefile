# set to 1 to use exact template matching semantics
EXACT=
# xdr_pack seems to be buggy on big db files. Needs to be set on command line
XDR=

# root directory for ecolab include files and libraries
ifeq ($(shell ls $(HOME)/usr/ecolab/include/ecolab.h),$(HOME)/usr/ecolab/include/ecolab.h)
ECOLAB_HOME=$(HOME)/usr/ecolab
else
ECOLAB_HOME=/usr/local/ecolab
endif


include $(ECOLAB_HOME)/include/Makefile

MODELS=etierra
OTHER_OBJS=CPUInst0.o Soup.o genebank.o neutrals.o complement.o
FLAGS+=-std=c++11 $(OPT) -DNO_HASH

UTILS=BDBmerge

ifdef MEMDEBUG
FLAGS+=-DRealloc=Realloc
endif

ifdef EXACT
FLAGS+=-DEXACT_TEMPLATE
endif

ifeq ($(OS),Linux)
ifdef GDBM_COMPAT
LIBS+=-lgdbm_compat
endif
LIBS+=-lgdbm
endif

ifdef AEGIS
aegis-all: all checkMissing
endif

without-xdr:
	$(MAKE) XDR= all

#chmod command is to counteract AEGIS removing execute privelege from scripts
all: $(MODELS) $(UTILS)
	-$(CHMOD) a+x *.tcl *.sh *.pl

# This rule uses a header file of object descriptors
$(MODELS:=.o): %.o: %.cc 
	$(CPLUSPLUS) -c $(FLAGS)  $<

# how to build a model executable
$(MODELS): %: %.o $(OTHER_OBJS) 
	$(LINK) $(FLAGS) $*.o $(OTHER_OBJS) $(MODLINK) $(LIBS) -o $@

$(UTILS): %: %.c
	$(CC) $(FLAGS) $< $(LIBS) -o $@

include $(MODELS:=.d) $(OTHER_OBJS:.o=.d)

clean:
	$(BASIC_CLEAN)
	rm -f $(MODELS) $(MODELS:%=%_classdesc.h) 
	rm -rf classdesc-lib classdesc-lib.cc classdesc.h classdesc.a
	rm -f $(UTILS)

checkMissing:
	sh test/checkMissing.sh
