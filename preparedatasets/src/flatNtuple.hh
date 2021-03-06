#ifndef FLAT_NTUPLE_H
#define FLAT_NTUPLE_H

#include <string> 
#include <vector> 
#include "ntuple_defaults.hh"
#include "writeNtuple_common.hh"
#include <boost/ptr_container/ptr_vector.hpp>
#include "TTree.h"
#include "TFile.h"

namespace bf { 
  const unsigned save_weight_ratios  = 1u << 0; 
  const unsigned save_category_trees = 1u << 1; 
  const unsigned save_category_hists = 1u << 2; 
  const unsigned default_save = save_weight_ratios; 
}

class TChain; 

class WtRatio 
{
public: 
  WtRatio(const double* num, const double* denom, double* prod); 
  void update(); 
private: 
  const double* m_num; 
  const double* m_denom; 
  double* m_prod; 
}; 

int flatNtuple(SVector input_files, 
	       Observers observers, 
	       std::vector<double> pt_cat_vec, 
	       std::string jetCollection = defopt::JCN, 
	       std::string jet_tagger = "JetFitterCharm", 
	       std::string output_file_name = "nothing.root", 
	       std::string weights_file = "", 
	       const unsigned flags = bf::default_save);

class WtRatioCtr : public std::vector<WtRatio> 
{ 
public: 
  void add(std::string base, TChain* input_chain, TTree& out_tree); 
  void add_bcu(const double* b, const double* c, const double* u, 
	       TTree& out_tree); 
  void update(); 
private: 
  boost::ptr_vector<double> m_doubles; 
}; 

void write_cat_trees(std::vector<double> pt_cat_vec, 
		     std::vector<double> eta_cat_vec,
		     TFile& output_file); 
void write_cat_hists(std::vector<double> pt_cat_vec, 
		     std::vector<double> eta_cat_vec,
		     TFile& output_file); 

  
#endif // FLAT_NTUPLE_H
