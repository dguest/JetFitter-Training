# makefile for JETNET ntuple preprocessor 
# Author: Dan Guest (dguest@cern.ch)
# Created: Sat Jan 28 13:09:53 CET 2012

# --- set dirs
BIN          := bin
SRC          := src
INC          := src

# --- external dirs 
EXT_SRC      := ext_src
EXT_INC      := ext_inc

#  set search path
vpath %.o    $(BIN)
vpath %.cxx  $(SRC) $(EXT_SRC)
vpath %.hh   $(INC) $(EXT_INC)

# --- load in root config
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTLDFLAGS   := $(shell root-config --ldflags)

# and py config
ifndef PY_CONFIG
PY_CONFIG := python-config
endif 
PY_FLAGS :=   $(shell $(PY_CONFIG) --includes)
PY_LIB   := -L$(shell $(PY_CONFIG) --prefix)/lib
PY_LIB   +=   $(shell $(PY_CONFIG) --libs)


# --- set compiler and flags
CXX          := g++
CXXFLAGS     := -O2 -Wall -fPIC -I$(INC) -I$(EXT_INC)
LDFLAGS      := -Wl,-no-undefined 

# rootstuff 
CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      := $(ROOTLDFLAGS)
LIBS         := $(ROOTLIBS)

# pystuff 
PY_LDFLAGS := $(LDFLAGS)
PY_LDFLAGS += $(PY_LIB)
PY_LDFLAGS += -shared

# ---- define objects
GEN_OBJ     := readJFBTagAna.o writeNtuple_Official.o
GEN_OBJ     += PtEtaCategoryTool.o
# GEN_OBJ     += writeNtupleAll.o readBaseBTagAnaTree.o
EXE_OBJ     := $(GEN_OBJ) main.o

PY_OBJ       := pyprep.o

ALLOBJ       := $(GEN_OBJ) $(PY_OBJ) main.o test.o

ALLOUTPUT    := pyprep.so # prepareAllNtuplesForNN

TEST_OBJ      := PtEtaCategoryTool.o test.o

all: $(ALLOUTPUT) test

prepareAllNtuplesForNN: $(EXE_OBJ:%=$(BIN)/%)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

test: $(TEST_OBJ:%=$(BIN)/%)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)


pyprep.so: $(GEN_OBJ:%=$(BIN)/%) $(PY_OBJ:%=$(BIN)/%)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(PY_LDFLAGS)

# --------------------------------------------------

# python object compile
$(BIN)/py%.o: py%.cxx 
	@echo compiling python object $@
	@mkdir -p $(BIN)
	@$(CXX) -c $(CXXFLAGS) $(PY_FLAGS) $< -o $@ 

# compile rule
$(BIN)/%.o: %.cxx
	@echo compiling $<
	@mkdir -p $(BIN)
	@$(CXX) -c $(CXXFLAGS) $< -o $@

# use auto dependency generation
DEP = $(BIN)

ifneq ($(MAKECMDGOALS),clean)
include  $(ALLOBJ:%.o=$(DEP)/%.d)
endif

$(DEP)/%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM $(CXXFLAGS) $< -o $@
	@sed -i 's,$*\.o,$(BIN)/& $@ ,g' $@

# clean
.PHONY : clean
CLEANLIST     = *~ *.o *.o~ *.d core 
clean:
	rm -fr $(CLEANLIST) $(CLEANLIST:%=$(BIN)/%) $(CLEANLIST:%=$(DEP)/%)
	rm -fr $(BIN) $(ALLOUTPUT)