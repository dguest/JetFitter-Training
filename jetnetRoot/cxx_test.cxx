#include "trainNN.hh"
#include "testNN.hh"

#include "cxx_test.hh"
#include <cassert> 

int main( int argn, char* argv[])
{ 
  assert(argn >= 1); 
  const char* input_file = argv[1]; 
  const char* output_class = "JetFitterNN"; 
  const char* output_dir = "test_weights"; 
  int n_iterations = 10; 
  int dilution_factor = 2; 
  bool use_sd = false; 
  bool with_ip3d = true; 
  int nodes_first_layer = 10; 
  int nodes_second_layer = 9; 
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
