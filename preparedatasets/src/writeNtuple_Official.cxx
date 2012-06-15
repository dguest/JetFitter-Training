#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "writeNtuple_Official.hh"
#include "writeNtuple_common.hh"
#include "BinTool.hh"
#include "TRandom.h" // next on list of things to kill
#include <cmath>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

//using namespace std;


int writeNtuple_Official(SVector input_files, 
			 Observers observers, 
			 std::vector<double> pt_cat_vec, 
			 std::string jetCollection,
			 std::string jet_tagger, 
			 std::string output_file, 
			 bool /*forNN*/) 
{


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
  
  // --- io trees 
  boost::scoped_ptr<TFile> file(new TFile(output_file.c_str(),"recreate"));
  boost::scoped_ptr<TTree> output_tree(new TTree("SVTree","SVTree"));

  // --- observer variables
  boost::ptr_vector<TChain> observer_chains; 
  boost::ptr_vector<double> observer_write_buffers; 
  boost::ptr_vector<int> int_observer_write_buffers;

  // ---- loop to set observer variables and chains
  for (SVector::const_iterator name_itr = observers.discriminators.begin(); 
       name_itr != observers.discriminators.end(); 
       name_itr++){ 
    std::cout << "instantiating " << *name_itr << endl;
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

    if (the_chain->GetEntries() == 0)
      throw std::runtime_error("could not load " + chain_name);

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = "discriminator" + *name_itr; 
    output_tree->Branch(name.c_str(), the_buffer); 

    // set the chain to write to this branch 
    the_chain->SetBranchStatus("*",0); 
    the_chain->SetBranchStatus("Discriminator",1); 
    the_chain->SetBranchAddress("Discriminator", the_buffer); 

  }


  // --- load jetfitter chain 
  std::string suffixJF("_" + jet_tagger + "/PerfTreeAll");
  std::cout << "instantiating " << suffixJF << endl;
  boost::scoped_ptr<TChain> treeJF
    (new TChain((jetCollection+suffixJF).c_str()));

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

  std::set<std::string> used_branches; 
  used_branches.insert("Flavour"); 
  used_branches.insert("JetPt"  ); 
  used_branches.insert("JetEta" ); 
  used_branches.insert("mass" ); 

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
  output_tree->Branch("mass",&mass); 
  output_tree->Branch("JetPt",&JetPt,"JetPt/D");
  output_tree->Branch("JetEta",&JetEta,"JetEta/D");

  // --- varaibles set in slimming 
  Int_t cat_flavour;
  Int_t bottom;
  Int_t charm;
  Int_t light;
  Double_t weight;
  Int_t cat_pT;
  Int_t cat_eta;

  output_tree->Branch("cat_pT",&cat_pT,"cat_pT/I");
  output_tree->Branch("cat_eta",&cat_eta,"cat_eta/I");
  output_tree->Branch("weight",&weight,"weight/D");

  output_tree->Branch("bottom",&bottom,"bottom/I");
  output_tree->Branch("charm",&charm,"charm/I");
  output_tree->Branch("light",&light,"light/I");


  output_tree->Branch("cat_flavour",&cat_flavour,"cat_flavour/I");  


  // --- observer variables 
  for (SVector::const_iterator name_itr = observers.double_variables.begin(); 
       name_itr != observers.double_variables.end(); 
       name_itr++){ 
    if (used_branches.count(*name_itr)) continue; 

    // define buffers in which to store the vars
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = *name_itr; 
    output_tree->Branch(name.c_str(), the_buffer); 

    // set the chain to write to this branch 
    treeJF->SetBranchStatus(name.c_str(),1); 
    treeJF->SetBranchAddress(name.c_str(), the_buffer); 
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
    output_tree->Branch(name.c_str(), the_buffer); 

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

  double correctionfactor=1;

  
  int numPtBins = pt_categories.size() + 1; 
  int numEtaBins = abs_eta_categories.size() + 1; 

  Double_t toleranceb=4;
  Double_t tolerancec=4;
  Double_t tolerancel=1;
  TRandom random;


  Double_t maxweightb=0;
  Double_t maxweightl=0;
  Double_t maxweightc=0;
  
  int n_total_bins = numPtBins * numEtaBins; 
  std::vector<double> weightsb(n_total_bins);
  std::vector<double> weightsl(n_total_bins);
  std::vector<double> weightsc(n_total_bins);

  std::vector<double> countb(n_total_bins);
  std::vector<double> countl(n_total_bins);
  std::vector<double> countc(n_total_bins);

  for (int i=0;i<numPtBins*numEtaBins;i++){
    weightsb[i]=0;
    weightsl[i]=0;
    weightsc[i]=0;
    countb[i]=0;
    countl[i]=0;
    countc[i]=0;
  }
    
  for (Long64_t i=0;i<num_entries;i++) {
      
    treeJF->GetEntry(i);
      
    if (mass < -100) continue;
      
    if (fabs(JetEta) > 2.5 || JetPt <= magic::min_jet_pt_gev)  
      continue;

    int actualpT = pt_categories.get_bin(JetPt); 
    int actualeta = abs_eta_categories.get_bin(fabs(JetEta));
      
    int flavour=abs(Flavour);
      
    switch (flavour){
    case 5:
      countb[actualpT+numPtBins*actualeta] += 1; 
      break;
    case 4:
      countc[actualpT+numPtBins*actualeta] += 1; 
      break;
    case 1:
      countl[actualpT+numPtBins*actualeta] += 1;
      break;
    }

  }
    
      
  for (int i=0;i<numPtBins*numEtaBins;i++){
    weightsb[i]= (Double_t)numberb / (Double_t)countb[i] ;
    weightsl[i]= (Double_t)numberl / (Double_t)countl[i] ;
    weightsc[i]= (Double_t)numberc / (Double_t)countc[i];

    if (weightsb[i]>maxweightb) maxweightb=weightsb[i];
    if (weightsl[i]>maxweightl) maxweightl=weightsl[i];
    if (weightsc[i]>maxweightc) maxweightc=weightsc[i];

  }
  

  

  std::cout << " maxweightb: " << maxweightb << " maxweightc: " << maxweightc 
	    << " maxweightl: " << maxweightl << endl;

  std::cout << "Total entries are: " << num_entries << endl;
  
  for (Int_t i = 0; i < num_entries; i++) {

    if (i % 50000 == 0 ) {
      std::cout << boost::format("\rprocessing event number %i (%.0f%%)")
	% i % (float(i)*100 / float(num_entries));
      std::cout.flush(); 
    }
    
    treeJF->GetEntry(i);

    if (fabs(JetEta) < 2.5 &&
	JetPt > magic::min_jet_pt_gev &&
	mass > -100) {

      cat_pT=pt_categories.get_bin(JetPt);
      cat_eta=abs_eta_categories.get_bin(fabs(JetEta)); 
      
      cat_flavour=abs(Flavour);

      bottom=0;
      charm=0;
      light=0;

      bool throwevent(false);

      switch (cat_flavour){
      case 5:
	bottom=1;
	weight=weightsb[cat_pT+numPtBins*cat_eta];
            
	if (weight<maxweightb/toleranceb){
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

	if (weight<maxweightc/tolerancec){
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
            
	if (weight<maxweightl/tolerancel){
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
        
      }

      if (throwevent) continue;

      //read the others only on demand (faster)
      for (unsigned short j = 0; j < observer_chains.size(); j++){ 
	observer_chains.at(j).GetEntry(i); 
      }

      output_tree->Fill();
      
    }
    
  }
  // event counter doesn't add newline
  std::cout << "\n"; 


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
  
