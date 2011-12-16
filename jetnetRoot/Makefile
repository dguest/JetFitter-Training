# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#

JETNET_LIB=jetnet.o
JETNET_SRC=jetnet_35.f

PYFLAGS := -lpython2.6 -shared -I/usr/include/python2.6/ 

ROOTLIBS=$(shell root-config --libs)
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

PY_LINKER_OPTS :=  -lTJetNet -L. -Wl,-rpath,.
# PY_LINKER_OPTS += -Wl,-no-undefined 


all:  libTJetNet.so pynn.so test.exe

%Dict.cxx: %.h
	@echo making dict $@
	@rootcint $@ -c $*.h	

T%Dict.o: T%Dict.cxx 
	@echo building dict $@
	@$(CC) $(COPT) $(INC) -c $< -o $@

T%.o: T%.cxx T%.h
	@echo building $@
	@$(CC) $(COPT) $(INC) -c $< -o $@


$(JETNET_LIB): $(JETNET_SRC) 
	@echo Building JetNet Library
	@$(FC) $(FOPT) -c $(JETNET_SRC) -o $(JETNET_LIB)

test.exe: $(T_OBJ) $(T_DICTS) $(JETNET_LIB)
	@echo making executable
	@$(CC) $(COPT) $(SRCS) $^ $(INC) $(LIBS) -o $@


pynn.so: pynn.cxx trainNN.cxx trainNN.hh
	@echo making python lib
	@$(CC) $^ $(PYFLAGS) -o $@ $(PY_LINKER_OPTS) $(INC)

libTJetNet.so pynn.cxx: trainNN.hh

libTJetNet.so: $(T_OBJ) $(T_DICTS) $(JETNET_LIB)
	@echo building $@
	@$(CC) -shared -o $@ $^ $(INC) $(LIBS) #-Wl,-soname,libTJetNet.so.0

clean:
	@$(RM) $(EXE) *~ $(JETNET_LIB) libTJetNet.so *.o *.bak core *Dict.h *Dict.cxx gauss.exe
backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean backup