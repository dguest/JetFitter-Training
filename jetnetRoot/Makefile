# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#
# Simplified by Daniel Guest (dguest@cern.ch)
# on: Fri Dec 16 21:00:00 CET 2011

# --- set dirs
BIN          := bin/
LIB          := #lib/ 

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
INC := $(ROOTCFLAGS)
LIBS := -lgfortran $(ROOTLIBS) 

COPT := -O2 -fPIC -g
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
CXX_OBJECTS += $(T_OBJ) $(T_DICTS)

LIB_OBJECTS := $(JETNET_OBJ)
LIB_OBJECTS += $(CXX_OBJECTS)

all:  $(LIB)libTJetNet.so $(LIB)pynn.so #cxx_test.exe # root_test.exe

%Dict.cxx: %.h LinkDef.h
	@echo making dict $@
	@rm -f $*Dict.h
	@rootcint $@ -c $*.h	

$(BIN)T%Dict.o: T%Dict.cxx 
	@echo compiling dict $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

$(BIN)T%.o: T%.cxx 
	@echo compiling $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

$(BIN)py%.o: py%.cxx 
	@echo compiling python object $@
	@$(CC) $(COPT) $(INC) -c $< -o $@ $(PY_FLAGS)

$(BIN)%.o: %.cxx 
	@echo compiling $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

$(BIN)$(JETNET_OBJ): $(JETNET_SRC) 
	@echo compiling $@
	@$(FC) $(FOPT) -c $(JETNET_SRC) -o $@

%.exe: $(BIN)%.o 
	@echo making executable $@
	@$(CC) $^ -o $@ $(INC) $(LIBS) $(LINKER_OPTS)

$(LIB)pynn.so: $(BIN)pynn.o 
#	@mkdir -p $(LIB)
	@echo "linking $^ --> $@"
	@$(CC) $< -o $@ $(PY_LINKER_OPTS) $(INC) $(LIBS) 

$(LIB)libTJetNet.so: $(LIB_OBJECTS:%.o=$(BIN)%.o)
#	@mkdir -p $(LIB)
	@echo "linking $^ --> $@"
	@$(CC) -shared -o $@ $^ $(INC) $(LIBS) #-Wl,-soname,libTJetNet.so.0

# --- auto dep
DEP = $(BIN)

ifneq ($(MAKECMDGOALS),clean)
include  $(CXX_OBJECTS:%.o=$(DEP)%.d)
endif

$(DEP)%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM -MG $(INC) $< -o $@
	@sed -i 's,$*\.o,$(BIN)& $@ ,g' $@

# --- cleanup 

# --- install 

ifdef USER_LIB
install: all 
	@mkdir -p $(USER_LIB)
	cp -f *.so $(USER_LIB)
endif

DIRTY_THINGS := *~ $(JETNET_OBJ) *.o *.bak core
DIRTY_THINGS += *Dict.h *Dict.cxx *.exe
DIRTY_THINGS += pynn.so libTJetNet.so

clean:
	@$(RM) $(DIRTY_THINGS)
	@$(RM) -rf $(DEP) $(BIN)

cleanlib: 
	@$(RM) *.so

backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean cleanlib backup all install