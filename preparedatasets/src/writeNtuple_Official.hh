#ifndef WRITE_NTUPLE_OFFICIAL_H
#define WRITE_NTUPLE_OFFICIAL_H

#include <TString.h>
#include <vector>
#include <string> 

using namespace std;

typedef std::vector<std::string> SVector; 

class LoadOfficialDSException {};

namespace defopt { 
  const std::string JCN = "AntiKt4TopoEMJets"; 
  const std::string OFN = "../reduceddatasets/"
    "reduceddataset_" + JCN + "_forNN.root"; 
}

int writeNtuple_Official(SVector input_files, 
			 SVector observer_discriminators,
			 std::string jetCollectionName = defopt::JCN, 
			 std::string output_file = defopt::OFN, 
			 std::string suffix = "AOD",
			 bool forNN = true,
			 bool randomize = false);
  
#endif // WRITE_NTUPLE_OFFICIAL_H 
