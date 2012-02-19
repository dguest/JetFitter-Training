# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#
# Simplified by Daniel Guest (dguest@cern.ch)
# on: Fri Dec 16 21:00:00 CET 2011

# --- set dirs
BIN          := bin
SRC          := src
INC          := include
DICT         := dict
COMMON_INC   := common_include

vpath %.cxx $(SRC)
vpath %.f   $(SRC)
vpath %.hh  $(INC) $(COMMON_INC)
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
ROOTLD_OPT := $(shell root-config --ldflags)

FC := gfortran
CC := g++
LIBS := -lgfortran $(ROOTLIBS) 

COPT := -O2 -fPIC -g -I$(INC) -I$(COMMON_INC)
FOPT := -ffixed-form -ffixed-line-length-none -g -m32

# LINKER_OPTS := -L$(LIB) -Wl,-rpath,$(LIB) 

HERE := $(shell pwd)
LIBS += -L$(HERE) -Wl,-rpath,$(HERE)
COMMON_LIBS := $(HERE)/../common
LIBS += -L$(COMMON_LIBS) -Wl,-rpath,$(COMMON_LIBS)


# --- define specific files
JETNET_OBJ := jetnet.o
JETNET_SRC := jetnet_35.f

T_OBJ := TNeuralDataSet.o TJetNet.o TTrainedNetwork.o TNetworkToHistoTool.o
T_DICTS := $(T_OBJ:.o=Dict.o)

JETNET_CXX_OBJECTS := trainNN.o testNN.o doNormalization.o 
JETNET_CXX_OBJECTS += makeTestNtuple.o readReducedDataset.o 
JETNET_CXX_OBJECTS += $(T_OBJ) $(T_DICTS)

TJETNET_OBJECTS := $(JETNET_OBJ)
TJETNET_OBJECTS += $(JETNET_CXX_OBJECTS)


CXX_OBJECTS := $(JETNET_CXX_OBJECTS) pynn.o

# --------------------------------------------
# -------- start here --------------
# --------------------------------------------

ALL_LIBS := $(LIB)libTJetNet.so
ALL_LIBS += $(LIB)pynn.so
all: $(ALL_LIBS) #cxx_test.exe # root_test.exe


# ---build rules

$(DICT)/%Dict.cxx: %.h LinkDef.h 
	@echo making dict $@
	@mkdir -p $(DICT)
	@rm -f $(DICT)/$*Dict.h $(DICT)/$*Dict.cxx 
	@rootcint $@ -c $(INC)/$*.h	
	@sed -i 's,#include "$(INC)/\(.*\)",#include "\1",g' $(DICT)/$*Dict.h

$(BIN)/T%Dict.o: $(DICT)/T%Dict.cxx 
	@mkdir -p $(BIN)
	@echo compiling dict $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@

$(BIN)/py%.o: py%.cxx 
	@mkdir -p $(BIN)
	@echo compiling python object $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@ $(PY_FLAGS)

$(BIN)/%.o: %.cxx 
	@mkdir -p $(BIN)
	@echo compiling $@
	@$(CC) $(COPT) $(ROOTCFLAGS) -c $< -o $@

$(BIN)/$(JETNET_OBJ): $(JETNET_SRC) 
	@mkdir -p $(BIN)
	@echo compiling $@
	@$(FC) $(FOPT) -c $< -o $@


# --------------------------------------------
# ------- build the final products ----------
# --------------------------------------------

OUTLIB_OPTS := -Wl,-no-undefined 
OUTLIB_OPTS += $(ROOTLD_OPT)
OUTLIB_OPTS += -lJetnetTools

EXE_OPTS := $(OUTLIB_OPTS)
EXE_OPTS += $(LIBS)
%.exe: $(BIN)/%.o 
	@echo making executable $@
	@$(CC) $^ -o $@ $(OUTLIB_OPTS)


# --- library TJetnet
JN_OUTLIB_OPTS := $(OUTLIB_OPTS)
JN_OUTLIB_OPTS += $(LIBS)
JN_OUTLIB_OPTS += -shared
libTJetNet.so: $(TJETNET_OBJECTS:%.o=$(BIN)/%.o)
	@echo "linking $^ --> $@"
	@$(CC) $^ -o $@ $(JN_OUTLIB_OPTS)  #-Wl,-soname,libTJetNet.so.0

# --- python extension 
PY_OUTLIB_OPTS := $(OUTLIB_OPTS)
PY_OUTLIB_OPTS += $(LIBS)
PY_OUTLIB_OPTS += $(PY_LIB)
PY_OUTLIB_OPTS += -lTJetNet
PY_OUTLIB_OPTS += -shared
pynn.so: $(BIN)/pynn.o libTJetNet.so
	@echo "linking $^ --> $@"
	@$(CC) $< -o $@ $(PY_OUTLIB_OPTS) 


# --- auto dep
DEP = $(BIN)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),rmdep)
include  $(CXX_OBJECTS:%.o=$(DEP)/%.d)
endif
endif

$(DEP)/%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM -I$(INC) $(ROOTCFLAGS) -I$(COMMON_INC) $< -o $@
	@sed -i 's,$*\.o,$(BIN)/& $@ ,g' $@

$(DEP)/%Dict.d: $(DICT)/%Dict.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM $(ROOTCFLAGS) -I$(INC)  $< -o $@
	@sed -i 's,$*\.o,$(BIN)/& $@ ,g' $@


# --- install 

ifdef USER_LIB
install: all 
	@echo "WARNING: install is broken"
	@mkdir -p $(USER_LIB)
	cp -f *.so $(USER_LIB)
endif

# --- cleanup 

clean: cleanlib
	@$(RM) -rf $(DEP) $(BIN) $(DICT)

cleanlib: 
	@$(RM) *.so

backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean cleanlib backup all install check-py rmdep

rmdep: 
	$(RM) -f $(DEP)/*.d