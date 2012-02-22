#ifndef MAKE_TEST_NTUPLE_H
#define MAKE_TEST_NTUPLE_H

#include <string>
#include <vector>
#include <ostream>

struct IONames
{
  std::string input_weights; 
  std::string reduced_dataset; 
  std::string output_file; 
  std::string output_tree; 
};



void makeTestNtuple(IONames io_names, bool debug = true);  

int nothing();

#endif // MAKE_TEST_NTUPLE_H
