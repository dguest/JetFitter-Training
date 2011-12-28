#include "trainNN.hh"
#include "testNN.hh"

#include "cxx_test.hh"
#include <cassert> 

int main( int argn, char* argv[])
{ 

  const char* input_file = "../reduceddatasets"
    "/reduceddataset_AntiKt4TopoEMJets_forNN.root";
  if (argn > 1) 
    input_file = argv[1]; 

  const char* output_class = "JetFitterNN"; 
  const char* output_dir = "test_weights"; 
  int n_iterations = 10; 
  int dilution_factor = 2; 
  bool use_sd = false; 
  bool with_ip3d = true; 
  int nodes_first_layer = 8; 
  int nodes_second_layer = 4; 
  int restart_training_from = 0; 
  // bool debug = false; 


  trainNN(input_file, 
	  output_class,
	  output_dir, 
	  n_iterations,
	  dilution_factor,
	  use_sd,
	  with_ip3d,
	  nodes_first_layer,
	  nodes_second_layer,
	  restart_training_from);
  
  return 0; 
}
