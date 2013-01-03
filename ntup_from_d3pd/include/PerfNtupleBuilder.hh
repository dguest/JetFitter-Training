#ifndef PERF_NTUPLE_BUILDER_H
#define PERF_NTUPLE_BUILDER_H

class TTree; 
class TFile; 

#include <string> 

#include <boost/noncopyable.hpp>

class PerfNtupleBuilder : public boost::noncopyable 
{ 
public: 
  PerfNtupleBuilder(std::string out_file_name = "", 
		    std::string out_tree_name = "SVTree", 
		    unsigned flags = 0); 
  ~PerfNtupleBuilder(); 
  void write_entry(); 
  double pt; 
  double eta; 
  int flavor; 
  double jfc_anti_b; 
  double jfc_anti_u; 
  double cnn_anti_b; 
  double cnn_anti_u; 
  double mv1_anti_b; 
  double mv1_anti_u; 
  double mv1c_anti_b; 
  double mv1c_anti_u; 
private: 
  void init_tree(); 
  void set_flavor_branches(int flavor); 
  int m_bottom; 
  int m_charm; 
  int m_light; 
  TFile* m_out_file; 
  TTree* m_out_tree; 
  const unsigned m_flags; 
}; 

#endif // PERF_NTUPLE_BUILDER_H
