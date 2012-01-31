#include "writeNtuple_Official.hh"

#include <string>
#include <vector>

int main (int narg, char* argv[]) { 

  std::vector<std::string> files;
  files.push_back("files/*.root*"); 

  std::vector<std::string> observers;
  observers.push_back("IP3D"); 
  observers.push_back("IP2D"); 
  observers.push_back("SV1"); 
  observers.push_back("COMB"); 

  writeNtuple_Official(files,observers);  



}
