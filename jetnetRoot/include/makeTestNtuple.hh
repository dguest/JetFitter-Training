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

struct CategoryVectors
{
  std::vector<double> pt; 
  std::vector<double> eta; 
  bool build_default_values(); 
};

std::ostream& operator<<(std::ostream&, const CategoryVectors&); 

void makeTestNtuple(IONames io_names, 
		    CategoryVectors category_vectors = CategoryVectors());  

int nothing();

#endif // MAKE_TEST_NTUPLE_H
