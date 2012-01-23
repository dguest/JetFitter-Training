#include "TChain.h"
#include "TFile.h"
#include <iostream> 
#include "root_test.hh"

int main( int argn, char* argv[])
{ 

  const char* input_file = "../reduceddatasets"
    "/reduceddataset_AntiKt4TopoEMJets_forNN.root";

  TChain* the_chain = new TChain("SVTree",""); 
  the_chain->Add(input_file); 
  std::cout << the_chain->GetEntries() << " entries in chain\n"; 
  
  return 0; 
}
