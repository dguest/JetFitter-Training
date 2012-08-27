#ifndef NN_FILE_CONVERTERS
#define NN_FILE_CONVERTERS

#include <string> 
#include <set>

class TFile; 

void hist_from_nn(std::string nn_file, 
		  std::string out_file, 
		  std::string nn_name); 
void nn_from_hist(std::string hist_file, 
		  std::string nn_file, 
		  std::string nn_name); 

namespace { 
  
  void copy_hists(TFile& from, TFile& to, 
		  std::set<std::string> exclude = std::set<std::string>()); 
}

#endif //NN_FILE_CONVERTERS
