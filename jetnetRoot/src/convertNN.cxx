#include "TTrainedNetwork.h"
#include "TNeuralNetwork.h"
#include "TFile.h"
#include "NNAdapters.hh"
#include <iostream>

int main(int narg, char* varg[]) { 
  if (narg < 2) { 
    std::cerr << "enter a file to convert\n"; 
    return -1; 
  }
  std::string out_file = "new_nn.root"; 
  if (narg == 3) { 
    out_file = varg[2]; 
  }
  if (narg > 3) { 
    std::cerr << "too many args\n"; 
  }
  TNeuralNetwork* converted = getOldTrainedNetwork(varg[1]);
  TFile output(out_file.c_str(),"recreate"); 
  output.WriteTObject(converted); 
}
