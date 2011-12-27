# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#
# Simplified by Daniel Guest (dguest@cern.ch)
# on: Fri Dec 16 21:00:00 CET 2011

JETNET_OBJ=jetnet.o
JETNET_SRC=jetnet_35.f

PYFLAGS := -lpython2.6 -shared -I/usr/include/python2.6/ 

# ROOTLIBS=$(shell root-config --libs)
ROOTLIBS := -L$(shell root-config --libdir) 
ROOTLIBS += -lCore -lRIO -lCint -lHist -lTree -lMatrix -lMathCore
ROOTCFLAGS=$(shell root-config --cflags)

SRCS=trainNN.cxx

EXE=test.exe
LIB=libTJetNet.so

T_OBJ := TNeuralDataSet.o TJetNet.o TTrainedNetwork.o TNetworkToHistoTool.o
T_DICTS := $(T_OBJ:.o=Dict.o)
FC := gfortran
CC := g++
INC := $(ROOTCFLAGS)
LIBS := -lgfortran $(ROOTLIBS) 

COPT=-O2 -fPIC
FOPT=-ffixed-form -ffixed-line-length-none -u -Wall -fPIC -O

PY_LINKER_OPTS := -L. -Wl,-rpath,.
PY_LINKER_OPTS += -Wl,-no-undefined 
PY_LINKER_OPTS += -lTJetNet


LIB_OBJECTS := $(JETNET_OBJ)
LIB_OBJECTS += trainNN.o testNN.o doNormalization.o 
LIB_OBJECTS += $(T_OBJ) $(T_DICTS)

all:  libTJetNet.so pynn.so # test.exe

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

test.exe: $(T_OBJ) $(T_DICTS) $(JETNET_OBJ) doNormalization.o
	@echo making executable
	@$(CC) $(COPT) $(SRCS) $^ $(INC) $(LIBS) -o $@ 

py%.o: py%.cxx 
	@echo building $@
	@$(CC) $(COPT) $(INC) -c $< -o $@ $(PYFLAGS)

pynn.so: pynn.o libTJetNet.so
	@echo making python lib
	@$(CC) $^ -o $@ $(PY_LINKER_OPTS) $(INC) $(LIBS) $(PYFLAGS)

libTJetNet.so: $(LIB_OBJECTS)
	@echo building $@
	@$(CC) -shared -o $@ $^ $(INC) $(LIBS) #-Wl,-soname,libTJetNet.so.0

DIRTY_THINGS := $(EXE) *~ $(JETNET_OBJ) libTJetNet.so *.o *.bak core
DIRTY_THINGS += *Dict.h *Dict.cxx gauss.exe
DIRTY_THINGS += pynn.so

clean:
	@$(RM) $(DIRTY_THINGS)
backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean backup