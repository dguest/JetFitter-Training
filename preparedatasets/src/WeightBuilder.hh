#ifndef WEIGHT_BUILDER_H
#define WEIGHT_BUILDER_H

#include <map> 
#include <string> 

class TH2D; 

class WeightBuilder { 
public: 
  WeightBuilder(const std::map<int,std::string>& flavor_to_branch, 
		std::string file_name, 
		std::string hist_base_name = "cal"); 
  ~WeightBuilder();
  double lookup(int flavor, double x, double y) const ; 
private: 
  std::map<int, TH2D*> m_weight_hists; 
}; 

#endif // WEIGHT_BUILDER_H
