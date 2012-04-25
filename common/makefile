# makefile for JETNET tools 
# Author: Dan Guest (dguest@cern.ch)
# Created: Sat Jan 28 13:09:53 CET 2012

# --- set dirs
BIN          := bin
SRC          := src
INC          := include


#  set search path
vpath %.o    $(BIN)
vpath %.cxx  $(SRC) 
vpath %.hh   $(INC) 

# --- load in root config
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTLDFLAGS   := $(shell root-config --ldflags)

# and py config
PY_CONFIG := python2.7-config

PY_FLAGS :=   $(shell $(PY_CONFIG) --includes)
PY_LIB   := -L$(shell $(PY_CONFIG) --prefix)/lib
PY_LIB   +=   $(shell $(PY_CONFIG) --libs)


# --- set compiler and flags
CXX          := g++
CXXFLAGS     := -O2 -Wall -fPIC -I$(INC) 
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
GEN_OBJ     := PtEtaCategoryTool.o
PY_OBJ       := pyparse.o

ALLOBJ       := $(GEN_OBJ) $(PY_OBJ) 

ALLOUTPUT    := libJetnetTools.so

all: $(ALLOUTPUT) 

libJetnetTools.so: $(ALLOBJ:%=$(BIN)/%)
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
ifneq ($(MAKECMDGOALS),rmdep)
include  $(ALLOBJ:%.o=$(DEP)/%.d)
endif
endif


$(DEP)/%.d: %.cxx
	@echo making dependencies for $<
	@mkdir -p $(DEP)
	@$(CXX) -MM $(CXXFLAGS) $< -o $@
	@sed -i 's,$*\.o,$(BIN)/& $@ ,g' $@

# clean
.PHONY : clean rmdep all 
CLEANLIST     = *~ *.o *.o~ *.d core 
clean:
	rm -fr $(CLEANLIST) $(CLEANLIST:%=$(BIN)/%) $(CLEANLIST:%=$(DEP)/%)
	rm -fr $(BIN) $(ALLOUTPUT)

rmdep: 
	rm -f $(DEP)/*.d