#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "flatNtuple.hh"
#include "BinTool.hh"
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
#include <boost/algorithm/string.hpp>
#include <stdexcept>
#include <set> 


int flatNtuple(SVector input_files, 
	       Observers observers, 
	       std::vector<double> pt_cat_vec, 
	       std::string jetCollection,
	       std::string jet_tagger, 
	       std::string output_file_name, 
	       const unsigned flags) 
{
  BinTool pt_categories(pt_cat_vec); 

  std::vector<double> eta_cat_vec; 
  eta_cat_vec.push_back(0.7); 
  eta_cat_vec.push_back(1.5); 
  BinTool abs_eta_categories(eta_cat_vec); 

  std::cout << " opening input files: \n"; 
  for (SVector::const_iterator itr = input_files.begin(); 
       itr != input_files.end(); 
       itr++){ 
    std::cout << *itr << "\n"; 
  }
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
    if (flags & bf::save_weight_ratios) { 
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

  // --- variables used in slimming
  // TODO: these probably aren't used in the flat ntuple, 
  //       get rid of the hardcoding 

  std::set<std::string> used_branches; 
  used_branches.insert("Flavour"); 
  used_branches.insert("JetPt"  ); 
  used_branches.insert("JetEta" ); 
  used_branches.insert("mass"   ); 

  int Flavour; 
  Double_t JetPt;
  Double_t JetEta;
  double mass; 

  treeJF->SetBranchStatus("*",0); 
  treeJF->SetBranchStatus("Flavour",1); 
  treeJF->SetBranchStatus("JetPt",1); 
  treeJF->SetBranchStatus("JetEta",1); 
  treeJF->SetBranchStatus("mass",1); 

  if (treeJF->SetBranchAddress("Flavour",&Flavour) ||
      treeJF->SetBranchAddress("JetPt"  ,&JetPt) ||
      treeJF->SetBranchAddress("JetEta" ,&JetEta) ||
      treeJF->SetBranchAddress("mass"   ,&mass) ) { 
    throw std::runtime_error("missing essential leaf"); 
  }

  // mass, pt, and eta all pass through 
  output_tree.Branch("mass",&mass); 
  output_tree.Branch("JetPt",&JetPt,"JetPt/D");
  output_tree.Branch("JetEta",&JetEta,"JetEta/D");


  // --- varaibles set in slimming 
  Int_t cat_flavour = 0;
  Int_t bottom      = 0;
  Int_t charm       = 0;
  Int_t light       = 0;
  Int_t cat_pT      = 0;
  Int_t cat_eta     = 0;

  output_tree.Branch("cat_pT",&cat_pT,"cat_pT/I");
  output_tree.Branch("cat_eta",&cat_eta,"cat_eta/I");

  output_tree.Branch("bottom",&bottom,"bottom/I");
  output_tree.Branch("charm",&charm,"charm/I");
  output_tree.Branch("light",&light,"light/I");
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
  

  //for the NN you need to get the number of b,c or light jets

  std::cout << "counting entries, will take a while\n"; 
  Int_t num_entries = treeJF->GetEntries();

  std::cout << "Total entries are: " << num_entries << std::endl;
  
  for (Int_t i = 0; i < num_entries; i++) {

    if (i % 500000 == 0 ) {
      std::cout << " processing event number " << i << std::endl;
    }
    
    treeJF->GetEntry(i);

    if (fabs(JetEta) < magic::max_jet_eta && 
	JetPt > magic::min_jet_pt_gev ) {

      cat_flavour = abs(Flavour);
      cat_pT=pt_categories.get_bin(JetPt);
      cat_eta=abs_eta_categories.get_bin(fabs(JetEta)); 

      bottom=0;
      charm=0;
      light=0;

      bool throwevent = false;

      switch (cat_flavour){
      case 5:
	bottom = 1;
	break;
      case 4:
	charm=1;
	break;
      case 1:
	light=1;
	break;

      default:
	throwevent=true;
	break;
          
      } // end flavor switch 

      if (throwevent) continue;

      //read the others only on demand (faster)
      for (unsigned short j = 0; j < observer_chains.size(); j++){ 
	observer_chains.at(j).GetEntry(i); 
	ratios.update(); 
      }

      output_tree.Fill();
      
    }
    
  }


  // --- save configuration in tree 
  std::cout << "done!\n";
  output_file.WriteTObject(&output_tree); 

  return 0; 
  
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
    likelihood_buffers[name] = buffer; 
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
