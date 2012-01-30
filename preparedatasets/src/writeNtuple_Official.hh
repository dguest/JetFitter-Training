#include <TString.h>
#include <vector>
#include <string> 

using namespace std;

typedef std::vector<std::string> SVector; 

class LoadOfficialDSException {};

void writeNtuple_Official(SVector input_files, 
			  SVector observer_discriminators,
                          std::string jetCollectionName = 
			  "Cone4H1TowerParticleJets",
                          std::string suffix = "AOD",
                          bool forNN = false,
                          bool randomize = false);
  
