#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "writeNtuple_byPt.hh"
#include "PtEtaCategoryTool.hh"
#include "TRandom.h"
#include <cmath>
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
		     std::string jetCollectionName,
		     std::string output_file, 
		     std::string suffix) 
{
  using namespace std;

  typedef std::vector<double>::const_iterator DItr; 

  // --- setup pt / eta categories
  std::vector<double> pt_cat_vec; 
  pt_cat_vec.push_back(25);
  pt_cat_vec.push_back(35);
  pt_cat_vec.push_back(50);
  pt_cat_vec.push_back(80);
  pt_cat_vec.push_back(120);
  pt_cat_vec.push_back(200);
  CategoryMap pt_categories(pt_cat_vec); 

  std::vector<double> eta_cat_vec; 
  eta_cat_vec.push_back(0.7); 
  eta_cat_vec.push_back(1.5); 
  CategoryMap abs_eta_categories(eta_cat_vec); 

  // --- setup observer variables (if they aren't given)
  bool built_observers = observers.build_default_values(); 
  if (built_observers){ 
    std::cout << "used some default observers:\n"; 
    std::cout << observers << std::endl;
  }

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
  boost::ptr_vector<TFile> output_files;
  boost::ptr_vector<TTree> output_trees;
  typedef boost::ptr_vector<TTree>::iterator TreeItr; 
 
  double min_value = -INFINITY; 
  std::string min_val_name = "under"; 
  for (DItr max_itr = pt_categories.begin(); 
       max_itr != pt_categories.end(); 
       max_itr++){ 
    int int_max = int(*max_itr); 
    std::string max_val_name = boost::lexical_cast<std::string>(int_max); 
    std::string file_name = "reduced_" + 
      min_val_name + "_" + max_val_name + ".root"; 
    output_files.push_back(new TFile(file_name.c_str(),"recreate")); 
    output_trees.push_back(new TTree("SVTree","SVTree")); 

    min_val_name = max_val_name; 
  }
  std::string file_name = "reduced_" + min_val_name + "_up.root"; 
  output_files.push_back(new TFile(file_name.c_str(), "recreate")); 
  output_trees.push_back(new TTree("SVTree","SVTree")); 

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

    if (the_chain->GetEntries() == 0)
      throw LoadOfficialDSException();

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = "discriminator" + *name_itr; 
    for (TreeItr tree_itr = output_trees.begin(); 
	 tree_itr != output_trees.end(); tree_itr++){ 
      tree_itr->Branch(name.c_str(), the_buffer); 
    }

    // set the chain to write to this branch 
    the_chain->SetBranchStatus("*",0); 
    the_chain->SetBranchStatus("Discriminator",1); 
    the_chain->SetBranchAddress("Discriminator", the_buffer); 

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

  treeJF->SetBranchAddress("Flavour",&Flavour); 
  treeJF->SetBranchAddress("JetPt"  ,&JetPt); 
  treeJF->SetBranchAddress("JetEta" ,&JetEta); 
  treeJF->SetBranchAddress("mass"   ,&mass); 

  // mass, pt, and eta all pass through 
  for (TreeItr tree_itr = output_trees.begin(); 
       tree_itr != output_trees.end(); 
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

  for (TreeItr tree_itr = output_trees.begin(); 
       tree_itr != output_trees.end(); 
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
    for (TreeItr tree_itr = output_trees.begin(); 
	 tree_itr != output_trees.end(); 
	 tree_itr++){ 
      tree_itr->Branch(name.c_str(), the_buffer); 
    }

    // set the chain to write to this branch 
    treeJF->SetBranchStatus(name.c_str(),1); 
    treeJF->SetBranchAddress(name.c_str(), the_buffer); 
  }

  // --- descrete observer variables 
  for (SVector::const_iterator name_itr = observers.int_variables.begin(); 
       name_itr != observers.int_variables.end(); 
       name_itr++){ 

    // define buffers in which to store the vars
    int* the_buffer = new int; 
    int_observer_write_buffers.push_back(the_buffer); 
    std::string name = *name_itr; 
    for (TreeItr tree_itr = output_trees.begin(); 
	 tree_itr != output_trees.end(); 
	 tree_itr++){ 
      tree_itr->Branch(name.c_str(), the_buffer); 
    }

    // set the chain to write to this branch 
    treeJF->SetBranchStatus(name.c_str(),1); 
    treeJF->SetBranchAddress(name.c_str(), the_buffer); 
  }

  //=======================================================
  //============ the real stuff starts here ============
  //=======================================================
  

  //for the NN you need to get the number of b,c or light jets

  std::cout << "counting entries, will take a while\n"; 
  Int_t num_entries=treeJF->GetEntries();

  int numberb=0;
  int numberc=0;
  int numberl=0;

  for (Long64_t i=0;i<num_entries;i++) {

    treeJF->GetEntry(i);
      
    if (mass > -100){
      if (abs(Flavour)==5){
	numberb+=1;
      }
      if (abs(Flavour)==4){
	numberc+=1;
      }
      if (abs(Flavour==1)){
	numberl+=1;
      }
    }
  }

  
  //now you have to calculate the weights...
  //(store them in a matrix for b,c or light jets...

  std::cout << " number of b found : " << numberb 
       << " c: " << numberc 
       << " l: " << numberl << endl;

  
  int numPtBins = pt_categories.size(); 
  int numEtaBins = abs_eta_categories.size(); 

  typedef std::vector< std::vector<double> > PtEtaDouble; 
  PtEtaDouble weightsb(numPtBins, std::vector<double>(numEtaBins, 0));
  PtEtaDouble weightsl(numPtBins, std::vector<double>(numEtaBins, 0));
  PtEtaDouble weightsc(numPtBins, std::vector<double>(numEtaBins, 0));

  PtEtaDouble countb(numPtBins, std::vector<double>(numEtaBins, 0));
  PtEtaDouble countl(numPtBins, std::vector<double>(numEtaBins, 0));
  PtEtaDouble countc(numPtBins, std::vector<double>(numEtaBins, 0));

  Double_t toleranceb=4;
  Double_t tolerancec=4;
  Double_t tolerancel=1;
  TRandom random;

  std::vector<double> maxweightb(numPtBins);
  std::vector<double> maxweightl(numPtBins);
  std::vector<double> maxweightc(numPtBins);
  
    
  for (Long64_t i=0;i<num_entries;i++) {
      
    treeJF->GetEntry(i);
      
    if (mass < -100) continue;
      
    if (fabs(JetEta) > magic::max_jet_eta || 
	JetPt <= magic::min_jet_pt_gev)  
      continue;

    int actualpT = pt_categories.get_category(JetPt); 
    int actualeta = abs_eta_categories.get_category(fabs(JetEta));
      
    int flavour = abs(Flavour);
      
    switch (flavour){
    case 5:
      countb.at(actualpT).at(actualeta) += 1; 
      break;
    case 4:
      countc.at(actualpT).at(actualeta) += 1; 
      break;
    case 1:
      countl.at(actualpT).at(actualeta) += 1; 
      break;
    }

  }
  
  
      
  for (int pt_cat = 0; pt_cat < numPtBins; pt_cat++){ 
    std::vector<double>& eta_weights_b = weightsb.at(pt_cat); 
    std::vector<double>& eta_weights_l = weightsl.at(pt_cat); 
    std::vector<double>& eta_weights_c = weightsc.at(pt_cat); 
    for (int eta_cat = 0; eta_cat < numEtaBins; eta_cat++){ 
      eta_weights_b.at(eta_cat) = (Double_t)numberb / (Double_t)countb[i];
      eta_weights_l.at(eta_cat) = (Double_t)numberl / (Double_t)countl[i] ;
      eta_weights_c.at(eta_cat) = (Double_t)numberc / (Double_t)countc[i];
    }

    if (weightsb[i]>maxweightb) maxweightb=weightsb[i];
    if (weightsl[i]>maxweightl) maxweightl=weightsl[i];
    if (weightsc[i]>maxweightc) maxweightc=weightsc[i];

  }
 
 
  

  std::cout << " maxweightb: " << maxweightb << " maxweightc: " << maxweightc 
	    << " maxweightl: " << maxweightl << endl;

  std::cout << "Total entries are: " << num_entries << endl;

  Int_t counter=0;
  for (Int_t i = 0; i < num_entries; i++) {

    if (counter % 500000 == 0 ) {
      std::cout << " processing event number " << 
	counter << " data event: " << i << " which was event n. " 
		<< std::endl;
    }
    
    counter += 1;
     
    treeJF->GetEntry(i);


    if (fabs(JetEta) < magic::max_jet_eta && 
	JetPt > magic::min_jet_pt_gev &&
	mass > -100) {

      cat_pT = pt_categories.get_category(JetPt);
      cat_eta = abs_eta_categories.get_category(fabs(JetEta)); 
      
      cat_flavour = abs(Flavour);

      bottom=0;
      charm=0;
      light=0;

      bool throwevent = false;

      switch (cat_flavour){
      case 5:
	bottom=1;
	weight=weightsb[cat_pT+numPtBins*cat_eta];

	if (weight < maxweightb/toleranceb){
	  if (random.Uniform()>weight*toleranceb/maxweightb){
	    throwevent=true;
	  }
	  weight=1.;//maxweightb/toleranceb;
	}
	else{
	  weight/=(maxweightb/toleranceb);
	}
          
	break;
      case 4:
	charm=1;
	weight=weightsc[cat_pT+numPtBins*cat_eta];

	if (weight < maxweightc/tolerancec){
	  if (random.Uniform()>weight*tolerancec/maxweightc){
	    throwevent=true;
	  }
	  weight=1.;//maxweightc/tolerancec;
	}
	else{
	  weight/=(maxweightc/tolerancec);
	}

	break;
      case 1:
	light=1;
	weight=weightsl[cat_pT+numPtBins*cat_eta];
            
	if (weight < maxweightl/tolerancel){
	  if (random.Uniform()>weight*tolerancel/maxweightl){
	    throwevent=true;
	  }
	  weight=1.;//maxweightl/tolerancel;
	}
	else {
	  weight/=(maxweightl/tolerancel);
	}
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

      output_tree->Fill();
      
    }
    
  }


  file->WriteTObject(output_tree.get()); 

  // --- save configuration in tree 
  typedef std::vector<double>::const_iterator DVecItr; 
  TTree pt_cat_config("pt_cat","pt_cat"); 
  double pt_val_buffer; 
  pt_cat_config.Branch("pt_gev",&pt_val_buffer); 
  for (DVecItr itr = pt_cat_vec.begin(); itr != pt_cat_vec.end(); itr++){ 
    pt_val_buffer = *itr; 
    pt_cat_config.Fill(); 
  }
  file->WriteTObject(&pt_cat_config); 

  TTree eta_cat_config("eta_cat","eta_cat"); 
  double eta_val_buffer; 
  eta_cat_config.Branch("abs_eta",&eta_val_buffer); 
  for (DVecItr itr = eta_cat_vec.begin(); itr != eta_cat_vec.end(); itr++){ 
    eta_val_buffer = *itr; 
    eta_cat_config.Fill(); 
  }
  file->WriteTObject(&eta_cat_config); 

  std::cout << "done!\n";

  return 0; 
  
}
  
FlavorCountPtEta::FlavorCountPtEta(size_t n_pt, size_t n_eta): 
  _n_pt(n_pt), 
  _n_eta(n_eta), 
  _is_computed(false)
{
  _pt_eta_number.assign(n_pt, std::vector<double>(n_eta, 0));
  _pt_eta_weight.assign(n_pt, std::vector<double>(n_eta, 0));
  _pt_count.assign(n_pt, 0); 
  _pt_max_weights.assign(n_pt, 0); 
}

void FlavorCountPtEta::increment(size_t n_pt, size_t n_eta)
{
  _is_computed = false; 
  _pt_eta_number.at(n_pt).at(n_eta) += 1; 
}

void FlavorCountPtEta::compute() 
{
  if ( _is_computed) return; 

  typedef std::vector< std::vector<double> > DMatrix; 
  typedef std::vector<double>::iterator DIter; 
  
  for (size_t pt_cat = 0; pt_cat < _n_pt; pt_cat++){ 
    DIter begin_eta_count = _pt_eta_count.at(pt_cat).begin(); 
    DIter end_eta_count = _pt_eta_count.at(pt_cat).end(); 

    double bin_total = std::accumulate(begin_eta, end_eta, 0.0); 
    _pt_count.at(pt_cat) = bin_total; 


    // *************** work do here *****************
    // for (){ 
      
    // }
  }

  _is_computed = true; 
}
