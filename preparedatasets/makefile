# --- load in root libraries
ROOTCFLAGS    := $(shell root-config --cflags)
ROOTLIBS      := $(shell root-config --libs)
ROOTLDFLAGS   := $(shell root-config --ldflags)

# --- set compiler and flags
CXX          := g++
CXXFLAGS     := -O2 -Wall -fPIC 

CXXFLAGS     += $(ROOTCFLAGS)
LDFLAGS      += $(ROOTLDFLAGS)
LIBS         += $(ROOTLIBS)

BIN          := bin
PYDIR        := pylib

ifndef PY_CONFIG
PY_CONFIG := python-config
endif 

PY_FLAGS := $(shell $(PY_CONFIG) --includes)

# ---- set search path
vpath %.cxx      src
vpath %.i        src
vpath %.o    $(BIN)
HPATH         = src #tools #$(BIN) 
CXXFLAGS     += $(HPATH:%=-I%)

# ---- define macros
FILL_OBJ     := readJFBTagAna.o readBaseBTagAnaTree.o writeNtuple_Official.o
FILL_OBJ     += writeNtupleAll.o main.o
ALLOBJ       += $(FILL_OBJ)

all: prepareAllNtuplesForNN $(PYDIR)/_prepntuple.so

prepareAllNtuplesForNN: $(FILL_OBJ:%=$(BIN)/%)
	@echo linking $^
	@$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

$(PYDIR)/_%.so: $(FILL_OBJ:%=$(BIN)/%) $(BIN)/%_wrap.o
	@$(CXX) -o $@ $^ -shared $(LIBS) $(LDFLAGS)

# --------------------------------------------------

# swig wrapper source
src/%_wrap.cxx: %.i
	@echo building wrapper $@
	@mkdir -p $(PYDIR)
	@swig -python -c++ -outdir $(PYDIR) $<

$(BIN)/%_wrap.o: src/%_wrap.cxx
	@echo compiling $<
	@mkdir -p $(BIN)
	@$(CXX) $(PY_FLAGS) -c $< -o $@

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
	@rm -f $@.tmp

# clean
.PHONY : clean
CLEANLIST     = *~ *.o *.o~ *.d core 
clean:
	rm -fr $(CLEANLIST) $(CLEANLIST:%=$(BIN)/%) $(CLEANLIST:%=$(DEP)/%)

