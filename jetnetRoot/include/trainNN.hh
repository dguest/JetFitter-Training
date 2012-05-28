#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include <string>
#include <ostream>

class JetNet; 
class TTree; 

#include "normedInput.hh"



// --- for external use 
struct FlavorWeights 
{
  double bottom; 
  double charm; 
  double light; 
}; 

namespace train { 
  const unsigned push_min_to_xtest    = 1u << 0; 
  const unsigned req_training_lt_min  = 1u << 1; 

  const unsigned giacintos = push_min_to_xtest | req_training_lt_min;
}

// --- for internal use
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
void copy_cat_trees(TFile& dest_file, const TFile& source_file); 

namespace train_nn { 
  const std::vector<InputVariableInfo> INPUT_VARS; 
  const std::vector<int> N_NODES; 
  const FlavorWeights FLAVOR_WEIGHTS = {1,1,5}; 
}

void dump_nn_settings(std::ostream& stream, const JetNet* jn); 
std::ostream& operator<<(std::ostream& in, const JetNet* jn); 

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
	     bool debug = true, 
	     const unsigned bit_flags = train::giacintos);


#endif // TRAIN_NN_H
