#ifndef WRITE_NTUPLE_OFFICIAL_H
#define WRITE_NTUPLE_OFFICIAL_H

#include <vector>
#include <string> 
#include <ostream>

using namespace std;

typedef std::vector<std::string> SVector; 

class LoadOfficialDSException {};

namespace magic { 
  const float min_jet_pt_gev = 15.0; 
}

namespace defopt { 
  const std::string JCN = "AntiKt4TopoEMJets"; 
  const std::string OFN = "../reduceddatasets/"
    "reduceddataset_" + JCN + "_forNN.root"; 
}

struct Observers { 
  SVector discriminators; 
  SVector double_variables; 
  SVector int_variables; 
  bool build_default_values(); 
}; 

std::ostream& operator<<(std::ostream&, const Observers&); 

int writeNtuple_Official(SVector input_files, 
			 Observers observers, 
			 std::string jetCollectionName = defopt::JCN, 
			 std::string output_file = defopt::OFN, 
			 std::string suffix = "AOD",
			 bool forNN = true,
			 bool randomize = false);
  
#endif // WRITE_NTUPLE_OFFICIAL_H 
