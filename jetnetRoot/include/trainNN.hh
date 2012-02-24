#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include <string>

#include "normedInput.hh"

struct FlavorWeights 
{
  double bottom; 
  double charm; 
  double light; 
}; 

namespace train_nn { 
  const std::vector<InputVariableInfo> INPUT_VARS; 
  const std::vector<int> N_NODES; 
  const FlavorWeights FLAVOR_WEIGHTS = {1,1,5}; 
}


void trainNN(std::string inputfile,
	     std::string out_dir = "weights", 
             int nIterations=10,
             int dilutionFactor=2,
             int restartTrainingFrom=0, 
	     std::vector<int> n_hidden_layer_nodes = train_nn::N_NODES, 
	     std::vector<InputVariableInfo> = train_nn::INPUT_VARS, 
	     FlavorWeights flavor_weights = train_nn::FLAVOR_WEIGHTS,  
	     bool debug = true);

#endif // TRAIN_NN_H
