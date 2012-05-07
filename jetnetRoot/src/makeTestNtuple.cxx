#include "makeTestNtuple.hh"
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <iostream>
#include "TRandom.h"
#include <cmath>
#include <vector>
#include <set>
#include <algorithm>
#include <TH1D.h>
#include "TTrainedNetwork.h"
#include "JetNet.hh"
#include <TVectorD.h>
#include <TMatrixD.h>
#include <string> 
#include "nnExceptions.hh"
#include "normedInput.hh"

using namespace std;


void makeTestNtuple(IONames io_names, bool debug)


{
  // gROOT->ProcessLine("#include <string>");

  std::string input_weights_name = io_names.input_weights; 
  std::string input_dataset_name = io_names.reduced_dataset;
  std::string output_file_name = io_names.output_file; 
  std::string output_tree_name = io_names.output_tree; 

  TFile input_weights_file(input_weights_name.c_str());
  TTrainedNetwork* trainedNetwork = dynamic_cast<TTrainedNetwork*>
    (input_weights_file.Get("TTrainedNetwork"));

  
  if (!trainedNetwork) {
    throw LoadNetworkException(); 
  }

  Int_t nLayers=2+trainedNetwork->getnHiddenLayerSize().size();
  
  cout << " Layers number: " << nLayers << endl;
  
  //BEGIN HERE
  
  Int_t* nneurons=new Int_t[4]; // TODO: auto_ptr
  nneurons[0]=trainedNetwork->getnInput();
  nneurons[1]=trainedNetwork->getnHiddenLayerSize()[0];
  nneurons[2]=trainedNetwork->getnHiddenLayerSize()[1];
  nneurons[3]=trainedNetwork->getnOutput();
  
  cout << " [0]: " <<  nneurons[0] << " [1]: " << nneurons[1] << 
    " [2]: " << nneurons[2] << " [3] :" << nneurons[3] << endl;
  
  Int_t nInput=trainedNetwork->getnInput();
  cout << " Input size: " << nInput;

  vector<Int_t> nHiddenLayerSize=trainedNetwork->getnHiddenLayerSize();
  Int_t nHidden=nHiddenLayerSize.size();
  
  for (Int_t o=0;o<nHidden;++o) {
      cout << " Hidden lay: " << o << " size: " << nHiddenLayerSize[o];
    }
  Int_t nOutput=trainedNetwork->getnOutput();
  
  cout << " Output size: " << nOutput << endl;
  
  
  //now calculate the value using:
  TVectorD** resultVector=new TVectorD*[nHidden+1];
  
  for (Int_t o=0;o<nHidden+1;++o) {
      int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;
      int sizePreviousLayer=(o==0)?nInput:nHiddenLayerSize[o-1];
      resultVector[o]=new TVectorD(sizeActualLayer);
    }

  std::vector<TVectorD*> thresholdVectors=trainedNetwork->getThresholdVectors();
  std::vector<TMatrixD*> weightMatrices=trainedNetwork->weightMatrices();
 
  //END HERE

  //CROSS CHECK HERE

  JetNet* jn = new JetNet( 3000, 7000, nLayers, nneurons );

  jn->SetUpdatesPerEpoch( 10 );
  jn->SetUpdatingProcedure( 0 );
  jn->SetErrorMeasure( 0 );
  jn->SetActivationFunction( 1 );
  jn->SetPatternsPerUpdate( 7000 );
  jn->SetLearningRate( 1. );
  jn->SetMomentum( 0.5 );
  jn->SetInitialWeightsWidth( 0.6 );
  jn->SetLearningRateDecrease( 0.999 );

  jn->Init();

  jn->readBackTrainedNetwork(trainedNetwork);

  //END CROSS CHECK HERE 



  cout << " file name to open is: " << input_dataset_name << endl;

  TFile file(input_dataset_name.c_str());

  TTree* simu = dynamic_cast<TTree*>(file.Get("SVTree"));
  if (! simu){ 
    throw LoadReducedDSException(input_dataset_name.c_str(), "SVTree"); 
  }


  std::string normalization_info_tree_name = norm::info_tree_name; 
  TTree* normalization_info = dynamic_cast<TTree*>
    (input_weights_file.Get(normalization_info_tree_name.c_str())); 

  InputVariableContainer in_var; 
  if ( normalization_info ){ 
    if (debug) { 
      std::cout << "reading normalization from \"" << 
	normalization_info_tree_name.c_str() << "\"\n";
    }
    in_var.build_from_tree(normalization_info, simu); 
  }
  else {
    std::cout << "WARNING: " << normalization_info_tree_name << 
      " not found using default hardcoded normalization values\n"; 
    in_var.set_hardcoded_defaults(simu); 
  }

  if (debug) { 
    std::cout << "input variables: \n"; 
    for (InputVariableContainer::const_iterator itr = in_var.begin(); 
	 itr != in_var.end(); itr++){ 
      std::cout << *itr << std::endl;
    }

  }


  // ------- setup output ------------


  TFile* outputFile = new TFile(output_file_name.c_str(),"recreate");
  if ( outputFile->IsZombie() ) { 
    throw WriteFileException(); 
  }

  TTree* myTree=new TTree(output_tree_name.c_str(),output_tree_name.c_str());

  Double_t NNb;
  Double_t NNc;
  Double_t NNu;

  myTree->Branch("NNb",&NNb,"NNb/D");
  myTree->Branch("NNc",&NNc,"NNc/D");
  myTree->Branch("NNu",&NNu,"NNu/D");

  std::set<std::string> nn_input_variables; 
  for (InputVariableContainer::const_iterator itr = in_var.begin(); 
       itr != in_var.end(); itr++){ 
    itr->add_passthrough_branch_to(myTree); 
    nn_input_variables.insert(itr->get_name()); 
  }

  InputVariableContainer pass_through_vars; 
  TObjArray* leaf_array = simu->GetListOfLeaves(); 
  int n_leafs = leaf_array->GetSize();
  for (int leaf_n = 0; leaf_n < n_leafs; leaf_n++){ 
    TLeaf* the_leaf = dynamic_cast<TLeaf*>(leaf_array->At(leaf_n)); 
    assert(the_leaf); 
    std::string leaf_name = the_leaf->GetName(); 
    if ( !nn_input_variables.count(leaf_name) ) { 
      pass_through_vars.add_variable(leaf_name, simu); 
    }
  }
  if (debug) std::cout << "pass-through variables: \n"; 

  for (InputVariableContainer::const_iterator 
	 itr = pass_through_vars.begin(); 
       itr != pass_through_vars.end(); itr++){ 
    itr->add_passthrough_branch_to(myTree); 

    if (debug) std::cout << *itr << std::endl;

  }



  
  Int_t num_entries = simu->GetEntries(); 

  cout << "Total entries are: " << num_entries << endl;

  for (Int_t i = 0;i < num_entries; ++i){


    if (i % 500000 == 0 ) {
      std::cout << " processing event number " << i << std::endl;
    }
    
    simu->GetEntry(i); 

    for (int var_num = 0; var_num < in_var.size(); var_num++){ 
      jn->SetInputs(var_num, in_var.at(var_num).get_normed() ); 
    }

    jn->Evaluate();

    NNb = jn->GetOutput(0);
    NNc = jn->GetOutput(1); 
    NNu = jn->GetOutput(2);
    
    myTree->Fill();
  }


  outputFile->WriteTObject(myTree); 

  // outputFile->Write();
  outputFile->Close();

  
}
  
