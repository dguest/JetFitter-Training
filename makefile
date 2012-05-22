# master makefile for JetFitter package
# Author: Dan Guest (dguest@cern.ch)
# Created: Tue Feb  7 17:08:15 CET 2012

all: training prepare common profile
	@git rev-parse HEAD > .REVISION_STAMP

prepare: common 
	@$(MAKE) -C preparedatasets

training: common 
	@$(MAKE) -C jetnetRoot

common: 
	@$(MAKE) -C common

profile:
	@$(MAKE) -C profile

clean: 
	@$(MAKE) -C preparedatasets clean 
	@$(MAKE) -C jetnetRoot clean 
	@$(MAKE) -C common clean 
	@$(MAKE) -C profile clean 


rmdep: 
	@$(MAKE) -C preparedatasets rmdep 
	@$(MAKE) -C jetnetRoot rmdep
	@$(MAKE) -C common rmdep 
	@$(MAKE) -C profile rmdep 


.PHONY: clean all rmdep common training prepare profile
