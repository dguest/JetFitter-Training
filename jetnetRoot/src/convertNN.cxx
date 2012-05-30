#include "TTrainedNetwork.h"
#include "TFlavorNetwork.h"
#include "TFile.h"
#include "NNAdapters.hh"
#include <iostream>

int main(int narg, char* varg[]) { 
  if (narg != 2) { 
    std::cerr << "enter a file to convert\n"; 
    return -1; 
  }
  TFlavorNetwork* converted = getOldTrainedNetwork(varg[1]);
  TFile output("new_nn.root","recreate"); 
  output.WriteTObject(converted); 
}
