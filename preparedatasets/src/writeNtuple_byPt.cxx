#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "writeNtuple_byPt.hh"
#include "BinTool.hh"
#include <cmath>
#include <cstdlib> // for rand, srand
#include <ctime> 
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lexical_cast.hpp>

//using namespace std;


int writeNtuple_byPt(SVector input_files, 
		     Observers observers, 
		     std::vector<double> pt_cat_vec, 
		     std::string jetCollectionName,
		     std::string output_dir, 
		     std::string suffix) 
{
  using namespace std;
  srand(time(0)); 

  // --- setup pt / eta categories
  BinTool pt_categories(pt_cat_vec); 

  std::vector<double> eta_cat_vec; 
  eta_cat_vec.push_back(0.7); 
  eta_cat_vec.push_back(1.5); 
  BinTool abs_eta_categories(eta_cat_vec); 

  // --- setup observer variables (if they aren't given)
  bool built_observers = observers.build_default_values(); 
  if (built_observers){ 
    std::cout << "used some default observers:\n"; 
    std::cout << observers << std::endl;
  }

  // TODO: move this concatenation outside this routine  
  std::string jetCollection = jetCollectionName + suffix;

  // run output 
  // std::ofstream out_stream("output.out", ios_base::out | ios_base::trunc); 

  std::cout << " opening input files: \n"; 
  for (SVector::const_iterator itr = input_files.begin(); 
       itr != input_files.end(); 
       itr++){ 
    std::cout << *itr << "\n"; 
  }
  std::cout << " processing to obtain: " << jetCollection 
       << " root file "  << endl;
  
  std::string baseBTag("BTag_");

  // --- io trees 
  typedef boost::ptr_vector<TTree>::iterator TreeItr; 
  OutputNtuples output(pt_cat_vec, output_dir); 

  // --- observer variables
  boost::ptr_vector<TChain> observer_chains; 
  boost::ptr_vector<double> observer_write_buffers; 
  boost::ptr_vector<int> int_observer_write_buffers;

  // ---- loop to set observer variables and chains
  for (SVector::const_iterator name_itr = observers.discriminators.begin(); 
       name_itr != observers.discriminators.end(); 
       name_itr++){ 
    std::cout << "instantiating " << *name_itr << endl;
    std::string chain_name = baseBTag + jetCollection + 
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
      std::cerr << "ERROR: " << chain_name << " is empty\n"; 
      throw LoadOfficialDSException();
    }

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = "discriminator" + *name_itr; 
    for (TreeItr tree_itr = output.trees.begin(); 
	 tree_itr != output.trees.end(); tree_itr++){ 
      tree_itr->Branch(name.c_str(), the_buffer); 
    }

    // set the chain to write to this branch 
    the_chain->SetBranchStatus("*",0); 
    the_chain->SetBranchStatus("Discriminator",1); 
    int error_code = 0; 
    error_code = the_chain->SetBranchAddress("Discriminator", the_buffer); 
    if (error_code) { 
      std::cerr << "ERROR: could not find Discriminator in chain: " << 
	the_chain->GetName() << " file: " << 
	*input_files.begin() << std::endl;
      throw MissingInputVariableException(); 
    }
  }


  // --- load jetfitter chain 
  std::string suffixJF("_JetFitterTagNN/PerfTreeAll");
  std::cout << "instantiating JetFitterTagNN " << endl;
  boost::scoped_ptr<TChain> treeJF
    (new TChain((baseBTag+jetCollection+suffixJF).c_str()));

  for (SVector::const_iterator in_file_itr = input_files.begin(); 
       in_file_itr != input_files.end(); 
       in_file_itr++){ 
    treeJF->Add(in_file_itr->c_str()); 
  }

  if (treeJF->GetEntries()==0) 
      throw LoadOfficialDSException();

  // --- variables used in slimming
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
    std::cerr << "ERROR: missing essential leaf\n"; 
    throw MissingInputVariableException(); 
  }

  // mass, pt, and eta all pass through 
  for (TreeItr tree_itr = output.trees.begin(); 
       tree_itr != output.trees.end(); 
       tree_itr++){ 
    tree_itr->Branch("mass",&mass); 
    tree_itr->Branch("JetPt",&JetPt,"JetPt/D");
    tree_itr->Branch("JetEta",&JetEta,"JetEta/D");
  }

  // --- varaibles set in slimming 
  Int_t cat_flavour;
  Int_t bottom;
  Int_t charm;
  Int_t light;
  Double_t weight;
  Int_t cat_pT;
  Int_t cat_eta;

  for (TreeItr tree_itr = output.trees.begin(); 
       tree_itr != output.trees.end(); 
       tree_itr++){
    tree_itr->Branch("cat_eta",&cat_eta,"cat_eta/I");
    tree_itr->Branch("weight",&weight,"weight/D");

    tree_itr->Branch("bottom",&bottom,"bottom/I");
    tree_itr->Branch("charm",&charm,"charm/I");
    tree_itr->Branch("light",&light,"light/I");
    tree_itr->Branch("cat_flavour",&cat_flavour,"cat_flavour/I");  
  }


  // --- observer variables 
  for (SVector::const_iterator name_itr = observers.double_variables.begin(); 
       name_itr != observers.double_variables.end(); 
       name_itr++){ 

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = *name_itr; 
    for (TreeItr tree_itr = output.trees.begin(); 
	 tree_itr != output.trees.end(); 
	 tree_itr++){ 
      tree_itr->Branch(name.c_str(), the_buffer); 
    }

    // set the chain to write to this branch 
    int error_code = 0; 
    treeJF->SetBranchStatus(name.c_str(),1); 
    error_code = treeJF->SetBranchAddress(name.c_str(), the_buffer); 
    if (error_code) { 
      std::cerr << "ERROR: could not find double leaf: " << name << " in " << 
	*input_files.begin() << std::endl; 
      throw MissingInputVariableException(); 
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
    for (TreeItr tree_itr = output.trees.begin(); 
	 tree_itr != output.trees.end(); 
	 tree_itr++){ 
      tree_itr->Branch(name.c_str(), the_buffer); 
    }

    // set the chain to write to this branch 
    treeJF->SetBranchStatus(name.c_str(),1); 
    int error_code = 0; 
    error_code = treeJF->SetBranchAddress(name.c_str(), the_buffer); 
    if (error_code) { 
      std::cerr << "ERROR: could not find int leaf: " << name << " in " << 
	*input_files.begin() << std::endl; 
      throw MissingInputVariableException(); 
    }
  }

  //=======================================================
  //============ the real stuff starts here ============
  //=======================================================
  

  //for the NN you need to get the number of b,c or light jets

  std::cout << "counting entries, will take a while\n"; 
  Int_t num_entries = treeJF->GetEntries();

  std::cout << "Total entries are: " << num_entries << endl;
  
  int numPtBins = pt_categories.size() + 1; 
  int numEtaBins = abs_eta_categories.size() + 1; 

  FlavorCountPtEta count_b(numPtBins, numEtaBins, 4); 
  FlavorCountPtEta count_c(numPtBins, numEtaBins, 4); 
  FlavorCountPtEta count_l(numPtBins, numEtaBins, 1); 
    
  for (Long64_t i = 0; i < num_entries; i++) {
      
    treeJF->GetEntry(i);
      
    if (mass < -100) continue;
      
    if (fabs(JetEta) > magic::max_jet_eta || 
	JetPt <= magic::min_jet_pt_gev)  
      continue;

    int cat_pT = pt_categories.get_bin(JetPt); 
    int cat_eta = abs_eta_categories.get_bin(fabs(JetEta));
      
    int flavour = abs(Flavour);
      
    switch (flavour){
    case 5:
      count_b.increment(cat_pT, cat_eta); 
      break;
    case 4:
      count_c.increment(cat_pT, cat_eta); 
      break;
    case 1:
      count_l.increment(cat_pT, cat_eta); 
      break;
    }

  }

  // --- calculate the weights
  count_b.compute(); 
  count_c.compute(); 
  count_l.compute(); 

  for (Int_t i = 0; i < num_entries; i++) {

    if (i % 500000 == 0 ) {
      std::cout << " processing event number " << i << std::endl;
    }
    
    treeJF->GetEntry(i);

    if (fabs(JetEta) < magic::max_jet_eta && 
	JetPt > magic::min_jet_pt_gev &&
	mass > -100) {

      cat_pT = pt_categories.get_bin(JetPt);
      cat_eta = abs_eta_categories.get_bin(fabs(JetEta)); 
      
      cat_flavour = abs(Flavour);

      bottom=0;
      charm=0;
      light=0;

      bool throwevent = false;

      switch (cat_flavour){
      case 5:
	bottom = 1;
	weight = count_b.get_weight(cat_pT, cat_eta); 
	break;
      case 4:
	charm=1;
	weight = count_c.get_weight(cat_pT, cat_eta); 
	break;
      case 1:
	light=1;
	weight = count_l.get_weight(cat_pT, cat_eta); 
	break;

      default:
	throwevent=true;
	break;
          
      } // end flavor switch 

      if (fabs(weight) < 0.0e-4) throwevent = true; 

      if (throwevent) continue;

      //read the others only on demand (faster)
      for (unsigned short j = 0; j < observer_chains.size(); j++){ 
	observer_chains.at(j).GetEntry(i); 
      }

      output.trees.at(cat_pT).Fill();
      
    }
    
  }


  // --- save configuration in tree 
  typedef std::vector<double>::const_iterator DVecItr; 
  TTree pt_cat_config("pt_cat","pt_cat"); 
  double pt_val_buffer; 
  pt_cat_config.Branch("pt_gev",&pt_val_buffer); 
  for (DVecItr itr = pt_cat_vec.begin(); itr != pt_cat_vec.end(); itr++){ 
    pt_val_buffer = *itr; 
    pt_cat_config.Fill(); 
  }

  TTree eta_cat_config("eta_cat","eta_cat"); 
  double eta_val_buffer; 
  eta_cat_config.Branch("abs_eta",&eta_val_buffer); 
  for (DVecItr itr = eta_cat_vec.begin(); itr != eta_cat_vec.end(); itr++){ 
    eta_val_buffer = *itr; 
    eta_cat_config.Fill(); 
  }

  std::cout << "done!\n";

  for (size_t i = 0; i < output.trees.size(); i++){ 
    TFile& file = output.files.at(i); 
    TTree& tree = output.trees.at(i); 

    file.WriteTObject(&tree); 
    file.WriteTObject(&pt_cat_config); 
    file.WriteTObject(&eta_cat_config); 
  }

  return 0; 
  
}
  
FlavorCountPtEta::FlavorCountPtEta(size_t n_pt, size_t n_eta, 
				   double tolerance ): 
  _n_pt(n_pt), 
  _n_eta(n_eta), 
  _tolerance(tolerance), 
  _is_computed(false)
{
  _pt_eta_count.assign(n_pt, std::vector<double>(n_eta, 0));
  _pt_eta_weight.assign(n_pt, std::vector<double>(n_eta, 0));
  _pt_count.assign(n_pt, 0); 
  _pt_max_weights.assign(n_pt, 0); 
}

void FlavorCountPtEta::increment(size_t n_pt, size_t n_eta)
{
  _is_computed = false; 
  _pt_eta_count.at(n_pt).at(n_eta) += 1; 
}

void FlavorCountPtEta::compute() 
{
  if ( _is_computed) return; 

  typedef std::vector<double>::iterator DIter; 
  
  for (size_t pt_cat = 0; pt_cat < _n_pt; pt_cat++){ 
    DIter begin_eta_count = _pt_eta_count.at(pt_cat).begin(); 
    DIter end_eta_count = _pt_eta_count.at(pt_cat).end(); 

    double bin_total = std::accumulate(begin_eta_count, end_eta_count, 0.0); 
    _pt_count.at(pt_cat) = bin_total; 

    for (size_t eta_cat = 0; eta_cat < _n_eta; eta_cat++){ 
      double weight = bin_total / _pt_eta_count.at(pt_cat).at(eta_cat); 
      _pt_eta_weight.at(pt_cat).at(eta_cat) = weight; 
    }
    
    std::vector<double>& weights = _pt_eta_weight.at(pt_cat); 
    double max_weight = *std::max_element(weights.begin(), weights.end()); 
    _pt_max_weights.at(pt_cat) = max_weight; 
  }

  _is_computed = true; 
}

double FlavorCountPtEta::get_weight(size_t cat_pt, size_t cat_eta) const
{
  assert(_is_computed); 

  double weight = _pt_eta_weight.at(cat_pt).at(cat_eta); 
  double max_weight = _pt_max_weights.at(cat_pt); 
  double random = double(rand()) / double(RAND_MAX); 

  if (weight < max_weight / _tolerance){
    if (random >  _tolerance * weight / max_weight){
      return 0; 
    }
    return 1; 
  }
  else{
    return weight / ( max_weight / _tolerance );
  }
  
}


OutputNtuples::OutputNtuples(std::vector<double> pt_cat_vec, 
			     std::string output_dir)
{
  typedef boost::ptr_vector<TTree>::iterator TreeItr; 
 
  std::string min_val_name = "under"; 
  std::string max_val_name = ""; 
  for (std::vector<double>::const_iterator max_itr = pt_cat_vec.begin(); 
       max_itr != pt_cat_vec.end(); 
       max_itr++){ 
    int int_max = int(*max_itr); 
    max_val_name = boost::lexical_cast<std::string>(int_max); 
    std::string file_name = output_dir + "/reduced_" + 
      min_val_name + "_" + max_val_name + ".root"; 
    files.push_back(new TFile(file_name.c_str(),"recreate")); 
    trees.push_back(new TTree("SVTree","SVTree")); 

    min_val_name = max_val_name; 
  }
  std::string file_name = output_dir + "/reduced_" + 
    min_val_name + "_up.root"; 

  files.push_back(new TFile(file_name.c_str(), "recreate")); 
  trees.push_back(new TTree("SVTree","SVTree")); 
}
 
