#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <iostream>
#include "flatNtuple.hh"
#include "BinTool.hh"
#include "WeightBuilder.hh"
#include <cmath>
// #include <cstdlib> // for rand, srand
// #include <ctime> 
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <set> 


int flatNtuple(SVector input_files, 
	       Observers observers, 
	       std::vector<double> pt_cat_vec, 
	       std::string jetCollection,
	       std::string jet_tagger, 
	       std::string output_file_name, 
	       std::string weights_file, 
	       const unsigned flags) 
{
  BinTool pt_categories(pt_cat_vec); 

  std::vector<double> eta_cat_vec; 
  eta_cat_vec.push_back(0.7); 
  eta_cat_vec.push_back(1.5); 
  BinTool abs_eta_categories(eta_cat_vec); 

  std::cout << " opening " << input_files.size() << " input files\n"; 
  std::cout << " processing to obtain: " << jetCollection
	    << " root file "  << std::endl;
  
  // --- io trees 
  typedef boost::ptr_vector<TTree>::iterator TreeItr; 
  TFile output_file(output_file_name.c_str(),"recreate"); 
  TTree output_tree("SVTree","SVTree"); // TODO: softcode
  if (!output_file.IsOpen() || output_file.IsZombie()) { 
    throw std::runtime_error("could not create output tree"); 
  }

  // --- observer variables
  boost::ptr_vector<TChain> observer_chains; 
  boost::ptr_vector<double> observer_write_buffers; 
  boost::ptr_vector<int> int_observer_write_buffers;
  
  WtRatioCtr ratios; 

  // ---- loop to set observer variables and chains
  for (SVector::const_iterator name_itr = observers.discriminators.begin(); 
       name_itr != observers.discriminators.end(); 
       name_itr++){ 
    std::cout << "instantiating " << *name_itr << std::endl;
    std::string chain_name = jetCollection + 
      "_" + *name_itr + "/PerfTreeAll"; 

    TChain* the_chain = new TChain(chain_name.c_str()); 
    observer_chains.push_back(the_chain); 

    // fill chains
    for (SVector::const_iterator in_file_itr = input_files.begin(); 
	 in_file_itr != input_files.end(); 
	 in_file_itr++){ 
      the_chain->Add(in_file_itr->c_str()); 
    }

    if (the_chain->GetEntries() == 0) { 
      throw std::runtime_error(chain_name + " is empty");
    }

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = "discriminator" + *name_itr; 
    output_tree.Branch(name.c_str(), the_buffer); 


    // set the chain to write to this branch 
    // TODO: allow this to load other variables too
    the_chain->SetBranchStatus("*",0); 
    the_chain->SetBranchStatus("Discriminator",1); 
    int error_code = 0; 
    error_code = the_chain->SetBranchAddress("Discriminator", the_buffer); 
    if (error_code) { 
      std::string location = "chain: " + std::string(the_chain->GetName()) + 
	" file: " + *input_files.begin();  
      throw std::runtime_error
	("could not find Discriminator in " + location); 
    }
    bool is_jf = (name_itr->find("JetFitter") != std::string::npos); 
    if (flags & bf::save_weight_ratios && is_jf) { 
      ratios.add(*name_itr, the_chain, output_tree); 
    }
  }


  // --- load jetfitter chain 
  std::string suffixJF("_" + jet_tagger + "/PerfTreeAll");
  std::cout << "instantiating " << jetCollection << suffixJF << std::endl;
  boost::scoped_ptr<TChain> treeJF
    (new TChain((jetCollection+suffixJF).c_str()));

  for (SVector::const_iterator in_file_itr = input_files.begin(); 
       in_file_itr != input_files.end(); 
       in_file_itr++){ 
    treeJF->Add(in_file_itr->c_str()); 
  }

  if (treeJF->GetEntries()==0) { 
    std::string tree_name = treeJF->GetName(); 
    throw std::runtime_error("could not load " + tree_name ); 
  }

  // --- variables used in slimming get passed through, make sure they aren't 
  //     set as observers
  
  std::set<std::string> used_branches; 
  used_branches.insert("Flavour"); 
  used_branches.insert("JetPt"  ); 
  used_branches.insert("JetEta" ); 

  // also ignore any variables we're already setting
  used_branches.insert("bottom"); 
  used_branches.insert("charm"); 
  used_branches.insert("light"); 
  used_branches.insert("weight"); 
  used_branches.insert("cat_pT"); 
  used_branches.insert("cat_eta"); 
  used_branches.insert("cat_flavour"); 

  int Flavour; 
  Double_t JetPt;
  Double_t JetEta;

  treeJF->SetBranchStatus("*",0); 
  treeJF->SetBranchStatus("Flavour",1); 
  treeJF->SetBranchStatus("JetPt",1); 
  treeJF->SetBranchStatus("JetEta",1); 

  if (treeJF->SetBranchAddress("Flavour",&Flavour) ||
      treeJF->SetBranchAddress("JetPt"  ,&JetPt) ||
      treeJF->SetBranchAddress("JetEta" ,&JetEta) ) { 
    throw std::runtime_error("missing essential leaf"); 
  }

  // mass, pt, and eta all pass through 
  output_tree.Branch("JetPt",&JetPt,"JetPt/D");
  output_tree.Branch("JetEta",&JetEta,"JetEta/D");


  // flavors 
  // TODO: consider adding taus into this mix? 
  std::map<int,std::string> flavor_to_branch; 
  flavor_to_branch[5] = "bottom"; 
  flavor_to_branch[4] = "charm"; 
  flavor_to_branch[1] = "light";

  std::map<int,int*> truth_branches;   

  for (std::map<int,std::string>::const_iterator 
	 itr = flavor_to_branch.begin(); 
       itr != flavor_to_branch.end(); 
       itr++) {
    // create the output branch 
    int* truth_branch = new int; 
    int_observer_write_buffers.push_back(truth_branch); 
    output_tree.Branch(itr->second.c_str(), truth_branch); 
    
    // save a pointer so we can zero the branch 
    truth_branches[itr->first] = truth_branch; 
  }

  WeightBuilder* flav_wt_ptr = 0; 
  if (weights_file.size() != 0) {  
    std::string base = "JetEta_vs_JetPt"; 
    flav_wt_ptr = new WeightBuilder(flavor_to_branch, weights_file, base); 
  }
  boost::scoped_ptr<WeightBuilder> flav_weights(flav_wt_ptr); 

  // --- varaibles set in slimming 
  double weight = 1; 
  Int_t cat_flavour = 0;
  Int_t cat_pT      = 0;
  Int_t cat_eta     = 0;

  output_tree.Branch("weight",&weight); 
  output_tree.Branch("cat_pT",&cat_pT,"cat_pT/I");
  output_tree.Branch("cat_eta",&cat_eta,"cat_eta/I");
  output_tree.Branch("cat_flavour",&cat_flavour,"cat_flavour/I");  


  // --- observer variables 
  for (SVector::const_iterator name_itr = observers.double_variables.begin(); 
       name_itr != observers.double_variables.end(); 
       name_itr++){ 
    if (used_branches.count(*name_itr)) continue; 

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = *name_itr; 
    output_tree.Branch(name.c_str(), the_buffer); 

    // set the chain to write to this branch 
    int error_code = 0; 
    treeJF->SetBranchStatus(name.c_str(),1); 
    error_code = treeJF->SetBranchAddress(name.c_str(), the_buffer); 
    if (error_code) { 
      throw std::runtime_error("could not find double leaf: " + name +
			       " in " + *input_files.begin() ); 
    }
  }

  // --- descrete observer variables 
  for (SVector::const_iterator name_itr = observers.int_variables.begin(); 
       name_itr != observers.int_variables.end(); 
       name_itr++){ 
    if (used_branches.count(*name_itr)) continue; 

    // define buffers in which to store the vars
    int* the_buffer = new int; 
    int_observer_write_buffers.push_back(the_buffer); 
    std::string name = *name_itr; 
    output_tree.Branch(name.c_str(), the_buffer); 

    // set the chain to write to this branch 
    treeJF->SetBranchStatus(name.c_str(),1); 
    int error_code = 0; 
    error_code = treeJF->SetBranchAddress(name.c_str(), the_buffer); 
    if (error_code) { 
      throw std::runtime_error(" could not find int leaf: " + name + " in " + 
			       *input_files.begin());
    }
  }

  //=======================================================
  //============ the real stuff starts here ============
  //=======================================================

  Int_t num_entries = treeJF->GetEntries();

  std::cout << "Total entries are: " << num_entries << std::endl;
  
  for (Int_t i = 0; i < num_entries; i++) {

    if (i % 50000 == 0 ) {
      std::cout << boost::format("\rprocessing event number %i (%.0f%%)")
	% i % (float(i)*100 / float(num_entries));
      std::cout.flush(); 
    }
    
    treeJF->GetEntry(i);

    if (fabs(JetEta) < magic::max_jet_eta && 
	JetPt > magic::min_jet_pt_gev ) {

      cat_flavour = abs(Flavour);

      // for now I guess we skip the taus... 
      if ( !truth_branches.count(cat_flavour) ) continue; 

      // zero truth branches (maybe this structure should be a vector<pair>)
      for (std::map<int,int*>::iterator tr_itr = truth_branches.begin(); 
	   tr_itr != truth_branches.end(); tr_itr++) { 
	*tr_itr->second = 0; 
      }
      *truth_branches[cat_flavour] = 1; 

      cat_pT=pt_categories.get_bin(JetPt);
      cat_eta=abs_eta_categories.get_bin(fabs(JetEta)); 

      
      // lookup weight 
      if (flav_weights) { 
	weight = flav_weights->lookup(cat_flavour, JetPt, JetEta); 
      }

      //read the others only on demand (faster)
      for (unsigned short j = 0; j < observer_chains.size(); j++){ 
	observer_chains.at(j).GetEntry(i); 
	ratios.update(); 
      }

      output_tree.Fill();
      
    }
    
  }
  std::cout << "\n"; 		// the event counter doesn't add newlines

  // --- save configuration in tree 

  if (flags & bf::save_category_trees) 
    write_cat_trees(pt_cat_vec, eta_cat_vec, output_file); 
  if (flags & bf::save_category_hists)
    write_cat_hists(pt_cat_vec, eta_cat_vec, output_file); 

  std::cout << "done!\n";
  output_file.WriteTObject(&output_tree); 

  return 0; 
  
}

void write_cat_trees(std::vector<double> pt_cat_vec, 
		     std::vector<double> eta_cat_vec,
		     TFile& output_file) { 
  typedef std::vector<double>::const_iterator DVecItr; 
  TTree pt_cat_config("pt_cat","pt_cat"); 
  double pt_val_buffer; 
  pt_cat_config.Branch("pt_gev",&pt_val_buffer); 
  for (DVecItr itr = pt_cat_vec.begin(); itr != pt_cat_vec.end(); itr++){ 
    pt_val_buffer = *itr; 
    pt_cat_config.Fill(); 
  }
  output_file.WriteTObject(&pt_cat_config); 

  TTree eta_cat_config("eta_cat","eta_cat"); 
  double eta_val_buffer; 
  eta_cat_config.Branch("abs_eta",&eta_val_buffer); 
  for (DVecItr itr = eta_cat_vec.begin(); itr != eta_cat_vec.end(); itr++){ 
    eta_val_buffer = *itr; 
    eta_cat_config.Fill(); 
  }
  output_file.WriteTObject(&eta_cat_config); 
}

void write_cat_hists(std::vector<double> pt_cat_vec, 
		     std::vector<double> eta_cat_vec,
		     TFile& output_file) { 
  TH1D pt_cat_config("pt_cat","pt_cat",pt_cat_vec.size(), 0, 1); 
  for (size_t category = 0; category < pt_cat_vec.size(); category++){ 
    pt_cat_config.SetBinContent(category + 1, pt_cat_vec.at(category)); 
  }
  output_file.WriteTObject(&pt_cat_config); 

  TH1D eta_cat_config("eta_cat","eta_cat",eta_cat_vec.size(), 0, 1); 
  for (size_t category = 0; category < eta_cat_vec.size(); category++){ 
    eta_cat_config.SetBinContent(category + 1, eta_cat_vec.at(category)); 
  }
  output_file.WriteTObject(&eta_cat_config); 

}


  
WtRatio::WtRatio(const double* num, const double* denom, double* prod): 
  m_num(num), 
  m_denom(denom), 
  m_prod(prod)
{ 
}

void WtRatio::update() { 
  *m_prod = log(*m_num / *m_denom); 
}


void WtRatioCtr::add(std::string base, TChain* the_chain, TTree& out_tree) { 

  const unsigned n_flav = 3; 
  std::string flavors[n_flav] = {"b","c","u"};
  
  typedef std::vector<std::string> SVec; 
  SVec flavor_tags(flavors, flavors + n_flav); 
  
  std::map<std::string,double*> likelihood_buffers; 
  for (SVec::const_iterator itr = flavor_tags.begin(); 
       itr != flavor_tags.end(); itr++) { 
    double* buffer = new double; 
    m_doubles.push_back(buffer); 
    std::string name = "Likelihood_" + *itr; 
    the_chain->SetBranchStatus(name.c_str(), 1); 
    int ec = the_chain->SetBranchAddress(name.c_str(), buffer); 
    if (ec) throw std::runtime_error("could not set " + name); 
    likelihood_buffers[*itr] = buffer; 
  }
  SVec ratios; 
  ratios.push_back("Bc"); 
  ratios.push_back("Bu"); 
  ratios.push_back("Cb"); 
  ratios.push_back("Cu"); 
  for (SVec::const_iterator itr = ratios.begin(); 
       itr != ratios.end(); itr++){
    std::string out_br_name = "log" + *itr + base; 
    double* prod_buffer = new double; 
    m_doubles.push_back(prod_buffer); 
    out_tree.Branch(out_br_name.c_str(), prod_buffer); 

    std::string lower_ratio_name = *itr; 
    boost::to_lower(lower_ratio_name); 
    std::string num_str = lower_ratio_name.substr(0,1); 
    std::string denom_str = lower_ratio_name.substr(1,1); 
    assert(likelihood_buffers.count(num_str)); 
    assert(likelihood_buffers.count(denom_str)); 
    
    WtRatio the_ratio(likelihood_buffers[num_str], 
		      likelihood_buffers[denom_str], 
		      prod_buffer); 
    push_back(the_ratio); 
  }

}

void WtRatioCtr::add_bcu(const double* b, const double* c, const double* u, 
			 TTree& out_tree) { 

  assert(false); 		// not totally figured out
  double* b_buffer = new double; 
  m_doubles.push_back(b_buffer); 
  out_tree.Branch("log_c_b", b_buffer); 
    
  WtRatio b_ratio(c, b, b_buffer); 
  push_back(b_ratio); 

  double* u_buffer = new double; 
  m_doubles.push_back(u_buffer); 
  out_tree.Branch("log_c_u", u_buffer); 

  WtRatio u_ratio(c, u, u_buffer); 
  push_back(u_ratio); 
  
}

void WtRatioCtr::update() { 
  for (iterator itr = begin(); itr != end(); itr++) { 
    itr->update(); 
  }
}
