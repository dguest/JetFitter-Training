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
LIBS         += -lboost_program_options

# add libTJetNet
LINKER_OPTS := -L$(USER_LIB) -Wl,-rpath,$(USER_LIB)
LINKER_OPTS += -Wl,-no-undefined 
LINKER_OPTS += -lTJetNet
LIBS        += $(LINKER_OPTS)

BIN          := bin
PYDIR        := pylib

# ifndef PY_CONFIG
# PY_CONFIG := python-config
# endif 

# PY_FLAGS  = $(shell $(PY_CONFIG) --includes)

# ---- set search path
SRC_PATH     := src ../jetnetRoot ../preparedatasets/src
vpath %.cxx  $(SRC_PATH)
vpath %.i        src
vpath %.o    $(BIN)
HPATH         = $(SRC_PATH)
CXXFLAGS     += $(HPATH:%=-I%)

# ---- define macros
FILL_OBJ     := readReducedDataset.o testJetNet.o
FILL_OBJ     += doTesting.o
ALLOBJ       += $(FILL_OBJ)

all: doTesting.exe

doTesting.exe: $(FILL_OBJ:%=$(BIN)/%)
	@echo "linking $^ --> $@"
	@$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)

# $(PYDIR)/_%.so: $(FILL_OBJ:%=$(BIN)/%) $(BIN)/%_wrap.o
# 	@echo linking $^
# 	@$(CXX) -o $@ $^ -shared $(LIBS) $(LDFLAGS)
# 	@touch $(PYDIR)/__init__.py

# --------------------------------------------------

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
CLEANLIST     = *~ *.o *.o~ *.d core *.exe
clean:
	rm -fr $(CLEANLIST) $(CLEANLIST:%=$(BIN)/%) $(CLEANLIST:%=$(DEP)/%)
	rm -fr $(PYDIR)
