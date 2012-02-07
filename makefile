# master makefile for JetFitter package
# Author: Dan Guest (dguest@cern.ch)
# Created: Tue Feb  7 17:08:15 CET 2012

all: training prepare 

prepare: 
	  @$(MAKE) -C preparedatasets

training: 
	  @$(MAKE) -C jetnetRoot
