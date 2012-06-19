#ifndef AUGMENT_TREE_H
#define AUGMENT_TREE_H

class TFlavorNetwork; 

#include <string> 
#include <vector> 
#include <set> 
#include <boost/shared_ptr.hpp>

namespace augment 
{
  const unsigned show_progress = 1u << 0; 
  
  const unsigned default_opts  = 0; 
}; 

int augment_tree(std::string file_name, 
		 std::string nn_file, 
		 std::string tree_name, 
		 std::string output_file, 
		 std::vector<std::string> int_vec, 
		 std::vector<std::string> double_vec, 
		 std::set<std::string> subset, 
		 std::string extension, 
		 int max_entries = -1, 
		 const unsigned options = augment::default_opts); 

boost::shared_ptr<TFlavorNetwork> get_nn(std::string file_name); 

#endif // AUGMENT_TREE_H
