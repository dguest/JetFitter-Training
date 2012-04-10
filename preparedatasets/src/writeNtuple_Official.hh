#ifndef WRITE_NTUPLE_OFFICIAL_H
#define WRITE_NTUPLE_OFFICIAL_H

#include <string> 
#include "ntuple_defaults.hh"
#include "writeNtuple_common.hh"

using namespace std;

int writeNtuple_Official(SVector input_files, 
			 Observers observers, 
			 std::vector<double> pt_cat_vec, 
			 std::string jetCollectionName = defopt::JCN, 
			 std::string output_file = defopt::OFN, 
			 std::string suffix = "AOD",
			 bool forNN = true);
  
#endif // WRITE_NTUPLE_OFFICIAL_H 
