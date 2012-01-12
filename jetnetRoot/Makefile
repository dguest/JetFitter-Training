# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#
# Simplified by Daniel Guest (dguest@cern.ch)
# on: Fri Dec 16 21:00:00 CET 2011

JETNET_OBJ := jetnet.o
JETNET_SRC := jetnet_35.f

ifndef PY_CONFIG
PY_CONFIG := python-config
endif 

PY_FLAGS := $(shell $(PY_CONFIG) --includes)
PY_LIB   := -L$(shell $(PY_CONFIG) --prefix)/lib
PY_LIB   += $(shell $(PY_CONFIG) --libs)

ROOTLIBS   := $(shell root-config --libs)
ROOTCFLAGS := $(shell root-config --cflags)
ROOTLD     := $(shell root-config --ldflags)

T_OBJ := TNeuralDataSet.o TJetNet.o TTrainedNetwork.o TNetworkToHistoTool.o
T_DICTS := $(T_OBJ:.o=Dict.o)
FC := gfortran
CC := g++
INC := $(ROOTCFLAGS)
LIBS := -lgfortran $(ROOTLIBS) 

COPT := -O2 -fPIC -g
FOPT := -ffixed-form -ffixed-line-length-none -g -m32

LINKER_OPTS := -L. -Wl,-rpath,. 
LINKER_OPTS += -Wl,-no-undefined 
LINKER_OPTS += -lTJetNet
LINKER_OPTS += $(ROOTLD)

PY_LINKER_OPTS := $(LINKER_OPTS)
PY_LINKER_OPTS += $(PY_LIB)
PY_LINKER_OPTS += -shared

CXX_OBJECTS := trainNN.o testNN.o doNormalization.o 
CXX_OBJECTS += $(T_OBJ) $(T_DICTS)

LIB_OBJECTS := $(JETNET_OBJ)
LIB_OBJECTS += $(CXX_OBJECTS)

all:  libTJetNet.so pynn.so cxx_test.exe # root_test.exe

%Dict.cxx: %.h
	@echo making dict $@
	@rm -f $*Dict.h
	@rootcint $@ -c $*.h	

T%Dict.o: T%Dict.cxx 
	@echo building dict $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

T%.o: T%.cxx T%.h
	@echo building $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

%.o: %.cxx %.hh
	@echo building $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

$(JETNET_OBJ): $(JETNET_SRC) 
	@echo Building JetNet Library
	@$(FC) $(FOPT) -c $(JETNET_SRC) -o $(JETNET_OBJ)

%.exe: %.o 
	@echo making executable $@
	@$(CC) $^ -o $@ $(INC) $(LIBS) $(LINKER_OPTS)

py%.o: py%.cxx 
	@echo building $@
	@$(CC) $(COPT) $(INC) -c $< -o $@ $(PY_FLAGS)

pynn.so: pynn.o libTJetNet.so
	@echo making python lib
	@$(CC) $< -o $@ $(PY_LINKER_OPTS) $(INC) $(LIBS) 

libTJetNet.so: $(LIB_OBJECTS)
	@echo building $@
	@$(CC) -shared -o $@ $^ $(INC) $(LIBS) #-Wl,-soname,libTJetNet.so.0

# --- auto dep
DEP = dep

ifneq ($(MAKECMDGOALS),clean)
include  $(CXX_OBJECTS:%.o=$(DEP)/%.d)
endif

$(DEP)/%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM -MG $(INC) $< -o $@
	@sed -i 's,$*\.o,& $@ ,g' $@

# --- cleanup 

DIRTY_THINGS := *~ $(JETNET_OBJ) *.o *.bak core
DIRTY_THINGS += *Dict.h *Dict.cxx *.exe
DIRTY_THINGS += pynn.so libTJetNet.so

clean:
	@$(RM) $(DIRTY_THINGS)
	@$(RM) -rf $(DEP)
backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean backup all