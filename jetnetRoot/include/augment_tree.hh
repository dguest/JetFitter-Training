#ifndef AUGMENT_TREE_H
#define AUGMENT_TREE_H

class TFlavorNetwork; 

#include <string> 
#include <vector> 
#include <boost/shared_ptr.hpp>

int augment_tree(std::string file_name, 
		 std::string nn_file, 
		 std::string tree_name, 
		 std::string output_file, 
		 std::vector<std::string> int_vec, 
		 std::vector<std::string> double_vec, 
		 std::string extension, 
		 int max_entries = -1); 

boost::shared_ptr<TFlavorNetwork> get_nn(std::string file_name); 

#endif // AUGMENT_TREE_H
