#include "TTree.h"
#include "TFile.h"
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include <iostream>
#include <TPad.h>
//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <math.h>
#include <stdexcept>
#include <cassert>
#include "TJetNet.h"
#include "normedInput.hh"
#include "TNetworkToHistoTool.h"

#include "TTrainedNetwork.h"
#include "testNN.hh"
#include "nnExceptions.hh"

#include <iostream>

#include "TMatrixD.h"
#include "TVectorD.h"


std::string flavor_to_string(Flavor flavor)
{
  switch (flavor){ 
  case LIGHT: 
    return "light"; 
  case CHARM: 
    return "charm"; 
  case BOTTOM: 
    return "bottom";
  default: 
    assert(false); 
  }
}

std::string sample_to_string(Sample sample)
{
  switch (sample){ 
  case TRAIN: 
    return "train"; 
  case TEST: 
    return "test"; 
  default: 
    assert(false); 
  }
}

void testNN(std::string inputfile,
	    std::string training_file,
	    int dilutionFactor,
	    bool useSD,
	    bool withIP3D, 
	    std::string out_file, 
	    bool debug) {

  double bweight=1;
  double cweight=1.;
  double lweight=5;
  std::string normalization_info_tree_name = norm::info_tree_name; 

  gROOT->SetStyle("Plain");

  std::cout << "starting with settings: " << std::endl;
  std::cout << " dilutionFactor: " << dilutionFactor << std::endl;
  std::cout << " useSD: " << (useSD==true?"yes":"no") << std::endl;
  std::cout << " withIP3D: " << (withIP3D==true?"yes":"no") << std::endl;
  
  // the requisite root calls
  gROOT->ProcessLine("#include <TTree.h>"); 
  gROOT->ProcessLine("#include <TFile.h>"); 

  TFile file(inputfile.c_str());
  TTree *simu = (TTree*)file.Get("SVTree");

  TTree* normalization_info = dynamic_cast<TTree*>
    (file.Get(normalization_info_tree_name.c_str())); 

  InputVariableContainer in_var; 
  if ( normalization_info ){ 
    in_var.build_from_tree(normalization_info, simu); 
  }
  else {
    std::cout << "WARNING: " << normalization_info_tree_name << 
      " not found using default hardcoded normalization values\n"; 

    typedef NormedInput<int> II; 
    typedef NormedInput<double> DI; 
    in_var.push_back(new II("nVTX"               , -0.30, 0.50, simu)); 
    in_var.push_back(new II("nTracksAtVtx"       , -1.00, 1.60, simu)); 
    in_var.push_back(new II("nSingleTracks"      , -0.20, 0.50, simu)); 
    in_var.push_back(new DI("energyFraction"     , -0.23, 0.33, simu)); 
    in_var.push_back(new DI("mass"               , - 974, 1600, simu)); 
    in_var.push_back(new DI("significance3d"     , -   7, 14.0, simu)); 
    in_var.push_back(new DI("discriminatorIP3D"  , - 6.3,  6.0, simu)); 
    in_var.push_back(new II("cat_pT"             , - 3.0,  3.0, simu)); 
    in_var.push_back(new II("cat_eta"            , - 1.0,  1.0, simu)); 
    
  }

  if (debug) { 
    std::cout << "input variables: \n"; 
    for (InputVariableContainer::const_iterator itr = in_var.begin(); 
	 itr != in_var.end(); itr++){ 
      std::cout << *itr << std::endl;
    }

  }

  // training variables
  Double_t        weight;
  Int_t           bottom;
  Int_t           charm;
  Int_t           light;

  simu->SetBranchAddress("weight",&weight);
  simu->SetBranchAddress("bottom",   &bottom);
  simu->SetBranchAddress("charm",   &charm);
  simu->SetBranchAddress("light",&light);


  // === above was from the top of the train routine

  TFile trained_network_file(training_file.c_str()); 
  
  TObject* trained_network_obj = trained_network_file.Get("TTrainedNetwork");
  TTrainedNetwork* trained_network = 
    dynamic_cast<TTrainedNetwork*>(trained_network_obj); 
  if (trained_network == 0){ 
    throw LoadNetworkException(); 
  }

  int n_inputs = trained_network->getnInput(); 
  std::vector<int> hidden_layer_size = trained_network->getnHiddenLayerSize();
  int n_outputs = trained_network->getnOutput(); 

  int n_layers = 2 + hidden_layer_size.size(); 
  int* n_neurons = new int[n_layers ]; 

  // setup layer configuration 
  n_neurons[0] = n_inputs; 
  int current_layer = 1; 
  for (std::vector<int>::const_iterator itr = hidden_layer_size.begin(); 
       itr != hidden_layer_size.end(); 
       itr++){
    n_neurons[current_layer] = *itr; 
    current_layer++; 
  }
  n_neurons[current_layer] = n_outputs; 

  // Hack to get the constructor working (I don't know why these are needed)
  int numberTestingEvents = 0; 
  int numberTrainingEvents = 0; 

  TJetNet* jn = new TJetNet( numberTestingEvents, 
  			     numberTrainingEvents, 
  			     n_layers, 
  			     n_neurons );

  jn->Init();
  jn->readBackTrainedNetwork(trained_network);

  std::cout << "read in network" << std::endl;

  // === below was from the end of the train routine 

  typedef std::vector<TH1F*> TruthContainer; 
  typedef std::vector<TruthContainer> DenomContainer; 
  typedef std::vector<DenomContainer> NumContainer; 
  typedef std::vector<NumContainer> SampleContainer; 

  std::vector<TH1F*> all_hists; 
  
  SampleContainer sample_container; 
  for (int sample = 0; sample < 2; sample++){ 
    std::string sample_name = sample_to_string(Sample(sample)); 
    NumContainer num_container; 
    

    for (int num = 0; num < 3; num++){ 
      std::string num_name = flavor_to_string(Flavor(num)); 
      DenomContainer denom_container; 

      // only do light and bottom 
      for (int denom = 0; denom < 3; denom++){ 

	std::string denom_name = flavor_to_string(Flavor(denom)); 
	TruthContainer truth_container; 

	for (int truth = 0; truth < 3; truth++){ 
	  std::string truth_name= flavor_to_string(Flavor(truth)); 

	  std::string full_name = truth_name + "s_" + num_name + "_over_" + 
	    denom_name + "_" + sample_name; 

	  TH1F* the_hist = 0; 
	  
	  // don't do flavors over themselves
	  if (num != denom) { 
	    the_hist = new TH1F(full_name.c_str(),full_name.c_str(), 
				50, -0.1, 1.1); 

	  }
	  truth_container.push_back(the_hist); 
	  all_hists.push_back(the_hist); 
				    
	}
	denom_container.push_back(truth_container); 
      }
      num_container.push_back(denom_container);
    }
    sample_container.push_back(num_container); 
  }
  

  for (Int_t i = 0; i < simu->GetEntries(); i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " First plot. Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor!=0&&i%dilutionFactor!=1) continue;
    
    simu->GetEntry(i);

    for (int var_num = 0; var_num < in_var.size(); var_num++){ 
      jn->SetInputs(var_num, in_var.at(var_num).get_normed() ); 
    }

    jn->Evaluate();

    float bvalue = jn->GetOutput(0);
    float cvalue = jn->GetOutput(1); 
    float lvalue = jn->GetOutput(2);

    // training sample is i % dilutionFactor == 0, 
    // testing  sample is i % dilutionFactor == 1
    NumContainer& num_container = sample_container.at(i % dilutionFactor); 
      
    // only do charm and bottom 
    for (int num = 0; num < 3; num++){ 
      
      float numerator = -1; 
      if (num == LIGHT) { 
	numerator = lvalue; 
      }
      else if (num == CHARM) { 
	numerator = cvalue; 
      }
      else if (num == BOTTOM) { 
	numerator = bvalue; 
      }
      else { 
	assert(false); 
      }

      // only do light and bottom 
      for (int denom = 0; denom < 3; denom++){ 
	if (num == denom) continue; 

	float denominator = 0; 
	if (denom == LIGHT){ 
	  denominator = lvalue + numerator; 
	}
	else if (denom == CHARM) { 
	  denominator = cvalue + numerator; 
	}
	else if (denom == BOTTOM) { 
	  denominator = bvalue + numerator; 
	}

	float output = numerator / denominator; 
	
	TruthContainer& truth_container = num_container.at(num).at(denom); 

	if (light == 1) { 
	  truth_container.at(LIGHT)->Fill(output); 
	}
	else if (charm == 1) { 
	  truth_container.at(CHARM)->Fill(output); 
	}
	else if (bottom == 1) { 
	  truth_container.at(BOTTOM)->Fill(output); 
	}
	else { 
	  assert(false); 
	}

      }

    }

  } // end of hist filling loop  

  TFile out_tfile(out_file.c_str(),"recreate"); 
  for (std::vector<TH1F*>::const_iterator hist_itr = all_hists.begin(); 
       hist_itr != all_hists.end(); 
       hist_itr++){
    if ( *hist_itr ) { 
      std::cout << "writing " << (*hist_itr)->GetName() << std::endl;
      out_tfile.WriteTObject(*hist_itr); 
    }
  }
  out_tfile.Close(); 

  delete n_neurons; 
  delete jn; 
  for (std::vector<TH1F*>::iterator hist_itr = all_hists.begin(); 
       hist_itr != all_hists.end(); 
       hist_itr++){
    delete *hist_itr; 
    *hist_itr = 0; 
  }


  
}

