# Makefile for ROOT - Jetnet Interface classes and test applications 
# Author: Vassil Verguilov
# Created: 22.01.2005 @ 17:52
#

JETNET_LIB=jetnet.o
JETNET_SRC=jetnet_35.f

PYFLAGS := -lpython2.6 -shared -I/usr/include/python2.6/ 

ROOTLIBS=$(shell root-config --glibs)
ROOTCFLAGS=$(shell root-config --cflags)

SRCS=trainNN.cxx

EXE=test.exe
LIB=libTJetNet.so

OBJS=$(JETNET_LIB) TNeuralDataSet.o TJetNet.o
SOCMD=ld
FC=gfortran
CC=g++
#CC=gcc
INC := $(ROOTCFLAGS)
LIBS=-lgfortran $(ROOTLIBS) # -lstdc++ -lgcc -lc 
# LIBS_SO=-lgfortran $(ROOTLIBS) # -lstdc++ -lgcc -lc
COPT=-O2 -fPIC
# -g
FOPT=-ffixed-form -ffixed-line-length-none -u -Wall -fPIC -O
# -g

all:  lib pynn.so

TNeuralDataSetDict.h TNeuralDataSetDict.cxx : TNeuralDataSet.cxx TNeuralDataSet.h
	@$(RM) TNeuralDataSetDict.*
	@echo Making Dictionary for TNeuralDataSet... by call
	@rootcint TNeuralDataSetDict.cxx -c TNeuralDataSet.h

TJetNetDict.h TJetNetDict.cxx : TJetNet.cxx TJetNet.h
	@$(RM) TJetNetDict.*
	@echo Making Dictionary for TJetNet by call
	@rootcint TJetNetDict.cxx -c TJetNet.h

TTrainedNetworkDict.h TTrainedNetworkDict.cxx : TTrainedNetwork.cxx TTrainedNetwork.h
	@$(RM) TTrainedNetworkDict.*
	@echo Making Dictionary for TTrainedNetwork by call
	@rootcint TTrainedNetworkDict.cxx -c TTrainedNetwork.h

TNetworkToHistoToolDict.h TNetworkToHistoToolDict.cxx : TNetworkToHistoTool.cxx TNetworkToHistoTool.h
	@$(RM) TNetworkToHistoToolDict.*
	@echo Making Dictionary for TNetworkToHistoTool by call
	@rootcint TNetworkToHistoToolDict.cxx -c TNetworkToHistoTool.h


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
	@$(CC) -c -o $@ $<
	@echo Making Dictionary for TTrainedNetwork
	@rootcint TTrainedNetworkDict.cxx -c TTrainedNetwork.h
	@$(CC) $(COPT) -c TTrainedNetworkDict.cxx $(INC) -o TTrainedNetworkDict.o
	@$(CC) $(COPT) -c TTrainedNetwork.cxx $(INC) -o TTrainedNetwork.o
	@$(CC) -c -o $@ $<
	@echo Making Dictionary for TNetworkToHistoTool
	@rootcint TNetworkToHistoToolDict.cxx -c TNetworkToHistoTool.h
	@$(CC) $(COPT) -c TNetworkToHistoToolDict.cxx $(INC) -o TNetworkToHistoToolDict.o
	@$(CC) $(COPT) -c TNetworkToHistoTool.cxx $(INC) -o TNetworkToHistoTool.o
	@$(CC) $(COPT) -c -o $@ $<

pynn.so: $(OBJS) $(SRCS)
	@echo making executable
	@$(CC) $(COPT) $(SRCS) $(JETNET_LIB) $(PYFLAGS) TNeuralDataSetDict.o TNeuralDataSet.o TJetNetDict.o TJetNet.o TTrainedNetwork.o TTrainedNetworkDict.o TNetworkToHistoTool.o TNetworkToHistoToolDict.o $(INC) $(LIBS) -o $@


lib: $(OBJS) $(SRCS)
	@$(CC) -shared -Wl,-soname,libTJetNet.so.0 -o libTJetNet.so $(JETNET_LIB) TNeuralDataSetDict.o TNeuralDataSet.o TJetNetDict.o TJetNet.o TTrainedNetwork.o TTrainedNetworkDict.o TNetworkToHistoTool.o TNetworkToHistoToolDict.o $(INC) $(LIBS_SO)
#-g


$(OBJS1) : %.o : %.cxx $(JETNET_SRC)
	@$(FC) $(FOPT) -c $(JETNET_SRC) -o $(JETNET_LIB)
	@$(CC) -c -o $@ $<

jetnet: $(OBJS1)
	@$(CC) $(SRCS) $(INC) $(LIBS) -s -o $@ $(OBJS1)

clean:
	@$(RM) $(EXE) *~ $(JETNET_LIB) libTJetNet.so *.o *.bak core *Dict.h *Dict.cxx gauss.exe
backup:
	@$(RM) backup.tar.gz
	@tar czvf backup.tar.gz *.cxx *.h *.c *.C Makefile* *.f

