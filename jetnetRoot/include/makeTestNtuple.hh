#ifndef MAKE_TEST_NTUPLE_H
#define MAKE_TEST_NTUPLE_H

#include <string>

void makeTestNtuple(std::string input_weights_name,
		    std::string input_dataset_name, 
		    std::string output_file_name, 
		    std::string output_tree_name);  

int nothing();

#endif // MAKE_TEST_NTUPLE_H
