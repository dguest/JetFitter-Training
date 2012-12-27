#include "TreeTranslator.hh"
#include "JetFactory.hh"
#include <vector>
#include <string> 

#include "TROOT.h"

int main(int narg, char* argv[]) { 
  std::vector<std::string> in_files; 
  for (int argn = 1; argn < narg; argn++) { 
    in_files.push_back(argv[argn]); 
  }
  
  // standard reminder that ROOT is a pile of shit
  gROOT->ProcessLine("#include <vector>"); 

  TreeTranslator translator("btagd3pd", "testfile.root"); 
  translator.add_collection("antikt4lctopo"); 
  translator.translate(in_files); 

  return 0; 
}
