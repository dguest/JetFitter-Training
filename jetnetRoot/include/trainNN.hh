#ifndef TRAIN_NN_H
#define TRAIN_NN_H

#include <string>
#include <ostream>

class JetNet; 
class TTree; 

#include "NormedInput.hh"



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
  const unsigned verbose              = 1u << 2; 
  const unsigned write_out_to_file    = 1u << 3; 
  const unsigned throw_on_warn        = 1u << 4; 
  const unsigned use_random_entries   = 1u << 5; 
  const unsigned shuffle_train_set    = 1u << 6; 

  const unsigned giacintos = push_min_to_xtest | req_training_lt_min;

  const unsigned defaults  = giacintos | throw_on_warn; 
}

struct TrainingInputs 
{
  std::string file; 
  std::string output_dir; 
  int n_iterations; 
  int restart_training_from; 
  int n_training_events; 
  
  int n_patterns_per_update; 
  float learning_rate; 
  float learning_rate_decrease; 
}; 

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
const float LEARNING_RATE = 0.5; 
const float LEARNING_RATE_DECREASE = 0.99; 
const int DILUTION_FACTOR = 2; 

bool is_flavor_tagged(const TeachingVariables&); 
float get_entry_weight(const TeachingVariables&, const FlavorWeights&); 
float adjusted_keep_prob(float old_prob, const FlavorWeights&); 
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
			const TeachingVariables& teach, 
			TTree* input_tree, 
			const TrainingSettings& settings, 
			const FlavorWeights&, 
			const unsigned bit_flags); 

int copy_training_events(std::ostream& stream, 
			 JetNet* jn, 
			 const InputVariableContainer& in_buffer, 
			 const TeachingVariables& teach, 
			 TTree* input_tree, 
			 const TrainingSettings& settings, 
			 const FlavorWeights&, 
			 const unsigned bit_flags); 

void setup_jetnet(JetNet* jn, const TrainingInputs& t); 


void trainNN(const TrainingInputs inputs, 
	     std::vector<int> n_hidden_layer_nodes = train_nn::N_NODES, 
	     std::vector<InputVariableInfo> = train_nn::INPUT_VARS, 
	     const FlavorWeights flavor_weights = train_nn::FLAVOR_WEIGHTS,  
	     const unsigned bit_flags = train::defaults);

class EntrySelector 
{ 
public: 
  EntrySelector(int target_entries, int total_entries); 
  void set_random(bool = true); 
  bool try_entry(); 
  int get_n_accepted() const; 
  void set_overbook_factor(float); 
private: 
  int m_needed_entries; 
  const int m_total_entries; 
  const int m_target_entries; 
  int m_tried_entries; 
  bool m_is_random; 
  float m_overbook_factor; 
}; 

#endif // TRAIN_NN_H
