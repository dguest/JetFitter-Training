#include <TString.h>
#include <vector>
#include <string> 

using namespace std;

typedef std::vector<std::string> SVector; 

class LoadOfficialDSException {};

void writeNtuple_Official(SVector input_files, 
			  SVector observer_discriminators,
                          std::string jetCollectionName = 
			  "AntiKt4TopoEMJets",  
                          std::string suffix = "AOD",
                          bool forNN = true,
                          bool randomize = false);
  
