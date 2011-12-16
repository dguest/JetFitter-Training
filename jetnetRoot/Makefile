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

OBJS := $(JETNET_LIB) TNeuralDataSet.o TJetNet.o
# T_OBJ := 
FC := gfortran
CC := g++
INC := $(ROOTCFLAGS)
LIBS := -lgfortran $(ROOTLIBS) 

COPT=-O2 -fPIC
FOPT=-ffixed-form -ffixed-line-length-none -u -Wall -fPIC -O


all:  libTJetNet.so pynn.so test.exe

$(OBJS): $(JETNET_SRC) 
	@echo Building JetNet Library
	@$(FC) $(FOPT) -c $(JETNET_SRC) -o $(JETNET_LIB)
	@$(RM) TNeuralDataSetDict.*
	@echo Making Dictionary for TNeuralDataSet...
	@rootcint TNeuralDataSetDict.cxx -c TNeuralDataSet.h
	@$(CC) $(COPT) -c TNeuralDataSetDict.cxx $(INC) -o TNeuralDataSetDict.o
	@$(CC) $(COPT) -c TNeuralDataSet.cxx $(INC) -o TNeuralDataSet.o
	@$(RM) TJetNetDict.*
	@echo Making Dictionary for TJetNet
	@rootcint TJetNetDict.cxx -c TJetNet.h
	@$(CC) $(COPT) -c TJetNetDict.cxx $(INC) -o TJetNetDict.o
	@$(CC) $(COPT) -c TJetNet.cxx $(INC) -o TJetNet.o
	@echo Making Dictionary for TTrainedNetwork
	@rootcint TTrainedNetworkDict.cxx -c TTrainedNetwork.h
	@$(CC) $(COPT) -c TTrainedNetworkDict.cxx $(INC) -o TTrainedNetworkDict.o
	@$(CC) $(COPT) -c TTrainedNetwork.cxx $(INC) -o TTrainedNetwork.o
	@echo Making Dictionary for TNetworkToHistoTool
	@rootcint TNetworkToHistoToolDict.cxx -c TNetworkToHistoTool.h
	@$(CC) $(COPT) -c TNetworkToHistoToolDict.cxx $(INC) -o TNetworkToHistoToolDict.o
	@$(CC) $(COPT) -c TNetworkToHistoTool.cxx $(INC) -o TNetworkToHistoTool.o

test.exe: $(OBJS) $(SRCS)
	@echo making executable
	@$(CC) $(COPT) $(SRCS) $(JETNET_LIB) TNeuralDataSetDict.o TNeuralDataSet.o TJetNetDict.o TJetNet.o TTrainedNetwork.o TTrainedNetworkDict.o TNetworkToHistoTool.o TNetworkToHistoToolDict.o $(INC) $(LIBS) -o $@

PY_LINKER_OPTS :=  -lTJetNet -L. -Wl,-rpath,.
# PY_LINKER_OPTS += -Wl,-no-undefined 

pynn.so: pynn.cxx trainNN.cxx trainNN.hh
	@echo making python lib
	@$(CC) $^ $(PYFLAGS) -o $@ $(PY_LINKER_OPTS) $(INC)

libTJetNet.so pynn.cxx: trainNN.hh

libTJetNet.so: $(OBJS) $(SRCS)
	@$(CC) -shared -o libTJetNet.so $(JETNET_LIB) TNeuralDataSetDict.o TNeuralDataSet.o TJetNetDict.o TJetNet.o TTrainedNetwork.o TTrainedNetworkDict.o TNetworkToHistoTool.o TNetworkToHistoToolDict.o $(INC) $(LIBS) #-Wl,-soname,libTJetNet.so.0
#-g



clean:
	@$(RM) $(EXE) *~ $(JETNET_LIB) libTJetNet.so *.o *.bak core *Dict.h *Dict.cxx gauss.exe
backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

.PHONY: clean backup