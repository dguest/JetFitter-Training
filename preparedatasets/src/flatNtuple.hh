#ifndef FLAT_NTUPLE_H
#define FLAT_NTUPLE_H

#include <string> 
#include <vector> 
#include "ntuple_defaults.hh"
#include "writeNtuple_common.hh"
// #include <boost/ptr_container/ptr_vector.hpp>
#include "TTree.h"
#include "TFile.h"

int flatNtuple(SVector input_files, 
	       Observers observers, 
	       std::string jetCollection = defopt::JCN, 
	       std::string output_file_name = "nothing.root");
  
#endif // FLAT_NTUPLE_H
