#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "flatNtuple.hh"
// #include "BinTool.hh"
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
#include <stdexcept>

//using namespace std;


int flatNtuple(SVector input_files, 
	       Observers observers, 
	       std::string jetCollection,
	       std::string output_file_name) 
{
  // using namespace std;
  // srand(time(0)); 

  // --- setup pt / eta categories
  // BinTool pt_categories(pt_cat_vec); 

  // std::vector<double> eta_cat_vec; 
  // eta_cat_vec.push_back(0.7); 
  // eta_cat_vec.push_back(1.5); 
  // BinTool abs_eta_categories(eta_cat_vec); 

  // --- setup observer variables (if they aren't given)
  // bool built_observers = observers.build_default_values(); 
  // if (built_observers){ 
  //   throw std::runtime_error("no observers given"); 
  // }


  // run output 
  // std::ofstream out_stream("output.out", ios_base::out | ios_base::trunc); 

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
  }


  // --- load jetfitter chain 
  std::string suffixJF("_JetFitterCharm/PerfTreeAll");
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
  Int_t cat_flavour;
  Int_t bottom;
  Int_t charm;
  Int_t light;
  // Double_t weight;
  // Int_t cat_pT;
  // Int_t cat_eta;


  output_tree.Branch("bottom",&bottom,"bottom/I");
  output_tree.Branch("charm",&charm,"charm/I");
  output_tree.Branch("light",&light,"light/I");
  output_tree.Branch("cat_flavour",&cat_flavour,"cat_flavour/I");  


  // --- observer variables 
  for (SVector::const_iterator name_itr = observers.double_variables.begin(); 
       name_itr != observers.double_variables.end(); 
       name_itr++){ 

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
	JetPt > magic::min_jet_pt_gev &&
	mass > -100) {

      cat_flavour = abs(Flavour);

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
      }

      output_tree.Fill();
      
    }
    
  }


  // --- save configuration in tree 
  std::cout << "done!\n";
  output_file.WriteTObject(&output_tree); 

  return 0; 
  
}
  
