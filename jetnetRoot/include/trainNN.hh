#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include "TString.h"
#include <string>

#include "normedInput.hh"

namespace train_nn { 
  const std::vector<InputVariableInfo> INPUT_VARS; 
}

void trainNN(TString inputfile,
	     std::string out_dir = "weights", 
             int nIterations=10,
             int dilutionFactor=2,
             bool useSD=false,
             bool withIP3D=true,
             int nodesFirstLayer=10,
             int nodesSecondLayer=9,
             int restartTrainingFrom=0, 
	     std::vector<InputVariableInfo> = train_nn::INPUT_VARS, 
	     bool debug = true);

#endif // TRAIN_NN_H
