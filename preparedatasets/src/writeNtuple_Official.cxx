#include <TChain.h>
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "writeNtuple_Official.hh"
#include "PtEtaCategoryTool.hh"
#include "TRandom.h"
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

//using namespace std;

struct number_pair
{
  int first;
  double second;
  number_pair(int p1, double p2)
    : first (p1), second (p2) {}
  bool operator< (const number_pair& other) const
  { return second > other.second; }
};



int writeNtuple_Official(SVector input_files, 
			 Observers observers, 
			 std::string jetCollectionName,
			 std::string output_file, 
			 std::string suffix,
			 bool forNN,
			 bool randomize) 
{

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
    output_tree->Branch(name.c_str(), the_buffer); 

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

  if (forNN){
    output_tree->Branch("cat_pT",&cat_pT,"cat_pT/I");
    output_tree->Branch("cat_eta",&cat_eta,"cat_eta/I");
    output_tree->Branch("weight",&weight,"weight/D");

    output_tree->Branch("bottom",&bottom,"bottom/I");
    output_tree->Branch("charm",&charm,"charm/I");
    output_tree->Branch("light",&light,"light/I");
  }

  output_tree->Branch("cat_flavour",&cat_flavour,"cat_flavour/I");  


  // --- observer variables 
  for (SVector::const_iterator name_itr = observers.double_variables.begin(); 
       name_itr != observers.double_variables.end(); 
       name_itr++){ 

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

  if (forNN) {

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
  }
  
  //now you have to calculate the weights...
  //(store them in a matrix for b,c or light jets...

  std::cout << " number of b found : " << numberb 
       << " c: " << numberc 
       << " l: " << numberl << endl;

  double correctionfactor=1;

  
  int numPtBins = pt_categories.size(); 
  int numEtaBins = abs_eta_categories.size(); 

  Double_t* weightsb=0;
  Double_t* weightsl=0;
  Double_t* weightsc=0;

  Double_t* countb=0;
  Double_t* countl=0;
  Double_t* countc=0;

  Double_t toleranceb=4;
  Double_t tolerancec=4;
  Double_t tolerancel=1;
  TRandom random;


  Double_t maxweightb=0;
  Double_t maxweightl=0;
  Double_t maxweightc=0;
  
  
  if (forNN) {
    
    weightsb=new Double_t[numPtBins*numEtaBins];
    weightsl=new Double_t[numPtBins*numEtaBins];
    weightsc=new Double_t[numPtBins*numEtaBins];

    countb=new Double_t[numPtBins*numEtaBins];
    countl=new Double_t[numPtBins*numEtaBins];
    countc=new Double_t[numPtBins*numEtaBins];

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

      pair<int,double> ptInfo;
      pair<int,double> etaInfo;

      ptInfo.first = pt_categories.get_category(JetPt); 
      ptInfo.second = 1; 	// MAGIC!
      
      etaInfo.first = abs_eta_categories.get_category(fabs(JetEta));
      etaInfo.second = 1; 	// MAGIC!

      int actualpT=ptInfo.first;
      int actualeta=etaInfo.first;
      
      int flavour=abs(Flavour);

      //      std::cout << " actualpT " << actualpT << " actualeta " << actualeta << endl;
      
      switch (flavour){
      case 5:
	countb[actualpT+numPtBins*actualeta]+=1./(ptInfo.second*etaInfo.second);
	break;
      case 4:
	countc[actualpT+numPtBins*actualeta]+=1./(ptInfo.second*etaInfo.second);
	break;
      case 1:
	countl[actualpT+numPtBins*actualeta]+=1./(ptInfo.second*etaInfo.second);
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
  
  }
  

  std::cout << " maxweightb: " << maxweightb << " maxweightc: " << maxweightc << 
    " maxweightl: " << maxweightl << endl;

  



  std::vector<number_pair> outputvalues;
  
  for (Int_t i=0;i<num_entries;i++) {
    outputvalues.push_back(number_pair(i,random.Uniform()));
  }
  
  if (randomize){
    
    std::cout << " Doing sorting... " << endl;
    std::sort (outputvalues.begin(), outputvalues.end());
    std::cout << " End sorting ... " << endl;
  }
  

  std::cout << "Total entries are: " << num_entries << endl;
  Int_t i=0;

  
  vector<number_pair>::const_iterator begin=outputvalues.begin();
  vector<number_pair>::const_iterator end=outputvalues.end();

  Int_t counter=0;
  for (vector<number_pair>::const_iterator iter=begin;iter!=end;++iter){
    i=(*iter).first;

    //take only every fifth data point
    if (!forNN){
      if (counter%5 != 0){
	counter+=1;
	continue;
      }
    }
    

    

    if (counter % 500000 == 0 ) {
      std::cout << " processing event number " << 
	counter << " data event: " << i << " which was event n. " 
		<< std::endl;
    }
    
    counter+=1;
     
    
    treeJF->GetEntry(i);


    if (fabs(JetEta) < 2.5 &&
	JetPt > magic::min_jet_pt_gev &&
	mass > -100) {

      cat_pT=pt_categories.get_category(JetPt);
      cat_eta=abs_eta_categories.get_category(fabs(JetEta)); 
      
      cat_flavour=abs(Flavour);
      if (forNN){
	bottom=0;
	charm=0;
	light=0;

	bool throwevent(false);

	switch (cat_flavour){
	case 5:
	  bottom=1;
	  weight=weightsb[cat_pT+numPtBins*cat_eta];

	  if (forNN){
            
	    if (weight<maxweightb/toleranceb){
	      if (random.Uniform()>weight*toleranceb/maxweightb){
		throwevent=true;
	      }
	      weight=1.;//maxweightb/toleranceb;
	    }
	    else{
	      weight/=(maxweightb/toleranceb);
	    }
	  } 
            
          
	  break;
	case 4:
	  charm=1;
	  weight=weightsc[cat_pT+numPtBins*cat_eta];

	  if (forNN){
	    if (weight<maxweightc/tolerancec){
	      if (random.Uniform()>weight*tolerancec/maxweightc){
		throwevent=true;
	      }
	      weight=1.;//maxweightc/tolerancec;
	    }
	    else{
	      weight/=(maxweightc/tolerancec);
	    }
	  }
          


	  break;
	case 1:
	  light=1;
	  weight=weightsl[cat_pT+numPtBins*cat_eta];

	  if (forNN){
            
	    if (weight<maxweightl/tolerancel){
	      if (random.Uniform()>weight*tolerancel/maxweightl){
		throwevent=true;
	      }
	      weight=1.;//maxweightl/tolerancel;
	    }
	    else {
	      weight/=(maxweightl/tolerancel);
	    }
            
	  }
          
	  break;

	default:
	  throwevent=true;
	  break;
          
        
	}
        

	if (throwevent) continue;

      }
      

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
  
bool Observers::build_default_values()
{ 
  bool init_doubles = false; 
  if (double_variables.size() == 0) { 
    double_variables.push_back("energyFraction"); 
    double_variables.push_back("significance3d"); 
    init_doubles = true; 
  }
  bool init_ints = false; 
  if (int_variables.size() == 0) { 
    int_variables.push_back("nVTX"); 
    int_variables.push_back("nTracksAtVtx"); 
    int_variables.push_back("nSingleTracks"); 
    
    init_ints = true; 
  }
  return init_ints || init_doubles;
}

std::ostream& operator<<(std::ostream& out, const Observers& v)
{
  typedef std::vector<std::string>::const_iterator SVecItr; 
  out << "discriminators: "; 
  for (SVecItr itr = v.discriminators.begin(); 
       itr != v.discriminators.end(); itr++){ 
    out << *itr << ", "; 
  }
  out << "continuous variables: "; 
  for (SVecItr itr = v.double_variables.begin(); 
       itr != v.double_variables.end(); itr++){ 
    out << *itr << ", "; 
  }
  out << "discrete variables: "; 
  for (SVecItr itr = v.int_variables.begin(); itr != v.int_variables.end(); 
       itr++){ 
    out << *itr << ", "; 
  }
  return out; 
}
