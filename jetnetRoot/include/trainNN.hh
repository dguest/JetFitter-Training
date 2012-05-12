#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include <string>
#include <ostream>

class JetNet; 
class TTree; 

#include "normedInput.hh"



// for external use 
struct FlavorWeights 
{
  double bottom; 
  double charm; 
  double light; 
}; 

// for internal use
struct TrainingSettings 
{
  int dilution_factor; 
  int n_testing_events; 
  int n_training_events; 
};
struct TeachingVariables 
{
  double        weight;
  int          bottom;
  int           charm;
  int           light;
}; 

const int N_PATTERNS_PER_UPDATE = 200;// || _2 = 200 (before 100) _3,_4=20

bool is_flavor_tagged(const TeachingVariables&); 

namespace train_nn { 
  const std::vector<InputVariableInfo> INPUT_VARS; 
  const std::vector<int> N_NODES; 
  const FlavorWeights FLAVOR_WEIGHTS = {1,1,5}; 
}

void dump_nn_settings(std::ostream& stream, const JetNet* jn); 

int copy_testing_events(std::ostream& stream, 
			JetNet* jn, 
			const InputVariableContainer& in_buffer, 
			TeachingVariables& teach, 
			TTree* input_tree, 
			const TrainingSettings& settings, 
			const FlavorWeights&); 

int copy_training_events(std::ostream& stream, 
			 JetNet* jn, 
			 const InputVariableContainer& in_buffer, 
			 TeachingVariables& teach, 
			 TTree* input_tree, 
			 const TrainingSettings& settings, 
			 const FlavorWeights&); 

void setup_jetnet(JetNet* jn); 


void trainNN(std::string inputfile,
	     std::string out_dir = "weights", 
             int nIterations=10,
             int dilutionFactor=2,
	     int restartTrainingFrom = 0, 
	     std::vector<int> n_hidden_layer_nodes = train_nn::N_NODES, 
	     std::vector<InputVariableInfo> = train_nn::INPUT_VARS, 
	     FlavorWeights flavor_weights = train_nn::FLAVOR_WEIGHTS,  
	     int n_training_events_target = -1, 
	     bool debug = true);


#endif // TRAIN_NN_H
