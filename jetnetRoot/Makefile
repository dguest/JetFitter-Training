# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#
# Simplified by Daniel Guest (dguest@cern.ch)
# on: Fri Dec 16 21:00:00 CET 2011

# --- set dirs
BIN          := bin/
LIB          := #lib/ 
SRC          := src
INC          := include
DICT         := dict

vpath %.cxx $(SRC)
vpath %.f   $(SRC)
vpath %.hh  $(INC)
vpath %.h  $(INC)
vpath %Dict.h $(DICT)
vpath %Dict.cxx $(DICT)

# --- get build options
ifndef PY_CONFIG
PY_CONFIG := python-config
endif 

PY_FLAGS := $(shell $(PY_CONFIG) --includes)
PY_LIB   := -L$(shell $(PY_CONFIG) --prefix)/lib
PY_LIB   += $(shell $(PY_CONFIG) --libs)

ROOTLIBS   := $(shell root-config --libs)
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLD     := $(shell root-config --ldflags)

FC := gfortran
CC := g++
LIBS := -lgfortran $(ROOTLIBS) 

COPT := -O2 -fPIC -g -I$(INC)
FOPT := -ffixed-form -ffixed-line-length-none -g -m32

# LINKER_OPTS := -L$(LIB) -Wl,-rpath,$(LIB) 

# ifndef USER_LIB
HERE := $(shell pwd)
# endif 
LINKER_OPTS += -L$(HERE) -Wl,-rpath,$(HERE)


LINKER_OPTS += -Wl,-no-undefined 
LINKER_OPTS += -lTJetNet
LINKER_OPTS += $(ROOTLD)

PY_LINKER_OPTS := $(LINKER_OPTS)
PY_LINKER_OPTS += $(PY_LIB)
PY_LINKER_OPTS += -shared

# --- define specific files
JETNET_OBJ := jetnet.o
JETNET_SRC := jetnet_35.f

T_OBJ := TNeuralDataSet.o TJetNet.o TTrainedNetwork.o TNetworkToHistoTool.o
T_DICTS := $(T_OBJ:.o=Dict.o)

CXX_OBJECTS := trainNN.o testNN.o doNormalization.o 
CXX_OBJECTS += makeTestNtuple.o readReducedDataset.o 
CXX_OBJECTS += $(T_OBJ) $(T_DICTS)

LIB_OBJECTS := $(JETNET_OBJ)
LIB_OBJECTS += $(CXX_OBJECTS)

ALL_LIBS := $(LIB)libTJetNet.so
ALL_LIBS += $(LIB)pynn.so

all: $(ALL_LIBS) #cxx_test.exe # root_test.exe


$(DICT)/%Dict.cxx: %.h LinkDef.h
	@echo making dict $@
	@mkdir -p $(DICT)
	@rm -f $(DICT)/$*Dict.h $(DICT)/$*Dict.cxx 
	@rootcint $@ -c $(INC)/$*.h	
	@sed -i 's,#include "include/\(.*\)",#include "\1",g' $(DICT)/$*Dict.h


$(BIN)T%Dict.o: T%Dict.cxx 
	@echo compiling dict $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@

$(BIN)T%.o: T%.cxx 
	@echo compiling $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@

$(BIN)py%.o: py%.cxx 
	@echo compiling python object $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@ $(PY_FLAGS)

$(BIN)%.o: %.cxx 
	@echo compiling $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@

$(BIN)$(JETNET_OBJ): $(JETNET_SRC) 
	@echo compiling $@
	@$(FC) $(FOPT) -c $^ -o $@

%.exe: $(BIN)%.o 
	@echo making executable $@
	@$(CC) $^ -o $@ $(ROOTCFLAGS) $(LIBS) $(LINKER_OPTS)

$(LIB)pynn.so: $(BIN)pynn.o 
#	@mkdir -p $(LIB)
	@echo "linking $^ --> $@"
	@$(CC) $^ -o $@ $(PY_LINKER_OPTS) $(ROOTCFLAGS) $(LIBS) 

$(LIB)libTJetNet.so: $(LIB_OBJECTS:%.o=$(BIN)%.o)
#	@mkdir -p $(LIB)
	@echo "linking $^ --> $@"
	@$(CC) -shared -o $@ $^ $(ROOTCFLAGS) $(LIBS) #-Wl,-soname,libTJetNet.so.0

# --- auto dep
DEP = $(BIN)

ifneq ($(MAKECMDGOALS),clean)
include  $(CXX_OBJECTS:%.o=$(DEP)%.d)
endif

$(DEP)%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM -I$(INC) $(ROOTCFLAGS) $< -o $@
	@sed -i 's,$*\.o,$(BIN)& $@ ,g' $@

$(DEP)%Dict.d: $(DICT)/%Dict.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM $(ROOTCFLAGS) -I$(INC) $< -o $@
	@sed -i 's,$*\.o,$(BIN)& $@ ,g' $@


# --- cleanup 

# --- install 

ifdef USER_LIB
install: all 
	@echo "WARNING: install is broken"
	@mkdir -p $(USER_LIB)
	cp -f *.so $(USER_LIB)
endif

DIRTY_THINGS := *~ $(JETNET_OBJ) *.o *.bak core
DIRTY_THINGS += *Dict.h *Dict.cxx *.exe 
DIRTY_THINGS += $(INC)/*Dict.h $(SRC)/*Dict.cxx 
DIRTY_THINGS += pynn.so libTJetNet.so

clean:
	@$(RM) $(DIRTY_THINGS)
	@$(RM) -rf $(DEP) $(BIN) $(DICT)

cleanlib: 
	@$(RM) *.so

backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean cleanlib backup all install check-py
