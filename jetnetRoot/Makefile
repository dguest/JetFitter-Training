# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#
# Simplified by Daniel Guest (dguest@cern.ch)
# on: Fri Dec 16 21:00:00 CET 2011

JETNET_OBJ=jetnet.o
JETNET_SRC=jetnet_35.f

PY_FLAGS := -shared -I/usr/include/python2.6/ 
PY_LIB   := -lpython2.6 -shared

# ifeq (ROOT_IS_BAD,1)
# ROOTLIBS := -L$(shell root-config --libdir) 
# ROOTLIBS += -lCore -lRIO -lCint -lHist -lTree -lMatrix -lMathCore
# else 
ROOTLIBS := $(shell root-config --libs)
# endif 
ROOTCFLAGS=$(shell root-config --cflags)

SRCS=trainNN.cxx

LIB=libTJetNet.so

T_OBJ := TNeuralDataSet.o TJetNet.o TTrainedNetwork.o TNetworkToHistoTool.o
T_DICTS := $(T_OBJ:.o=Dict.o)
FC := gfortran
CC := g++
INC := $(ROOTCFLAGS)
LIBS := -lgfortran $(ROOTLIBS) 
# LIBS += -lstdc++ -lgcc -lc

COPT :=-O2 -fPIC -g
FOPT :=-ffixed-form -ffixed-line-length-none -g #-u -Wall -fPIC -O 
# FOPT +=-fdefault-real-8 -fdefault-double-8 -fno-align-commons

LINKER_OPTS := -L. -Wl,-rpath,.
LINKER_OPTS += -Wl,-no-undefined 
LINKER_OPTS += -lTJetNet
PY_LINKER_OPTS := $(LINKER_OPTS)
PY_LINKER_OPTS += $(PY_LIB)

LIB_OBJECTS := $(JETNET_OBJ)
LIB_OBJECTS += trainNN.o testNN.o doNormalization.o 
LIB_OBJECTS += $(T_OBJ) $(T_DICTS)

all:  libTJetNet.so pynn.so cxx_test.exe root_test.exe

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
	@echo making executable
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

DIRTY_THINGS := $(LIB) *~ $(JETNET_OBJ) *.o *.bak core
DIRTY_THINGS += *Dict.h *Dict.cxx *.exe
DIRTY_THINGS += pynn.so

clean:
	@$(RM) $(DIRTY_THINGS)
backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean backup