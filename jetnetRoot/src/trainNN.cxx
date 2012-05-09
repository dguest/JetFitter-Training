#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>

#include <string.h>
#include <cmath>
#include <sstream>
#include <cstdlib> // rand
#include <ctime> // to seed srand
#include "JetNet.hh"
#include "doNormalization.hh"
#include "NetworkToHistoTool.hh"

#include "normedInput.hh"
#include "nnExceptions.hh"

#include "TTrainedNetwork.h"
#include "NNAdapters.hh"
#include "trainNN.hh"

#include "OwnerVector.hh"

#include <iostream>

#include "TMatrixD.h"
#include "TVectorD.h"


using namespace std;


void trainNN(std::string inputfile,
	     std::string out_dir, 
             int nIterations,
             int dilutionFactor,
	     int restartTrainingFrom, 
	     std::vector<int> n_hidden_layer_nodes, 
	     std::vector<InputVariableInfo> input_variables, 
	     FlavorWeights flavor_weights, 
	     int n_training_events_target, 
	     bool debug) {

  srand(time(0)); 
  printf("--- starting trainNN ----\n"); 
  double bweight = flavor_weights.bottom;
  double cweight = flavor_weights.charm;
  double lweight = flavor_weights.light;

  if (n_hidden_layer_nodes.size() == 0){
    std::cout << "WARNING: setting hidden layers to default sizes\n"; 
    n_hidden_layer_nodes.push_back(15); 
    n_hidden_layer_nodes.push_back(8); 
  }
  int nodesFirstLayer = n_hidden_layer_nodes.at(0); 
  int nodesSecondLayer = n_hidden_layer_nodes.at(1);


  gROOT->ProcessLine("#include <TTree.h>"); 
  gROOT->ProcessLine("#include <TFile.h>"); 
  
  cout << "starting with settings: " << endl;
  cout << " nIterations: " << nIterations << endl;
  cout << " dilutionFactor: " << dilutionFactor << endl;
  cout << " nodesFirstLayer: " << nodesFirstLayer << endl;
  cout << " nodesSecondLayer: " << nodesSecondLayer << endl;
  
  
  TFile *file= new TFile(inputfile.c_str());
  TTree *simu = dynamic_cast<TTree*>(file->Get("SVTree"));
  if (! simu){ 
    std::cerr << "ERROR, could not find SVTree in " << inputfile << 
      std::endl; 
    throw LoadReducedDSException(); 
  }

  InputVariableContainer in_var; 
  if (input_variables.size() == 0) { 
    std::cout << "WARNING: no input variables given, using defaults\n"; 
    in_var.set_hardcoded_defaults(simu); 
  }
  else { 
    for (std::vector<InputVariableInfo>::const_iterator 
	   itr = input_variables.begin(); 
	 itr != input_variables.end(); 
	 itr++){
      in_var.add_variable(*itr, simu); 
    }
  }

  if (debug){ 
    std::cout << "input variables: \n"; 
    for (InputVariableContainer::const_iterator itr = in_var.begin(); 
	 itr != in_var.end(); itr++){ 
      std::cout << " " << *itr << std::endl;
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

  int* nneurons;

  if (nodesSecondLayer != 0){
    nneurons=new int[4];
  }
  else {
    nneurons=new int[3];
  }


  int numberinputs = in_var.size();
  nneurons[0] = numberinputs;

  int nlayer=3;
  if (nodesSecondLayer != 0){
    nlayer=4;
  }
  
  nneurons[1]=nodesFirstLayer;

  if (nodesSecondLayer!=0)  {
      nneurons[2]=nodesSecondLayer;
      nneurons[3]=3;
  }
  else {
    nneurons[2]=3;
  }

  //  float eventWeight(0);
  float trainingError(0);
  float testError(0);
  
  //setting learning parameters

  cout << " now providing training events " << endl;
  
  Int_t numberTrainingEvents=0;
  Int_t numberTestingEvents=0;

  int n_entries = simu->GetEntries(); 
  std::cout << n_entries << " entries in chain\n"; 

  if (n_training_events_target > 0) { 
    int target_entries = min(n_training_events_target, 
			     n_entries / dilutionFactor); 
    numberTrainingEvents = target_entries; 
    numberTestingEvents = target_entries; 
  }
  else { 
    for (Int_t i = 0; i < n_entries; i++) {

      if (i % 100000 == 0 ) {
	std::cout << " Counting training / testing events in sample."
	  " Looping over event " << i << std::endl;
      }
    
      if (i%dilutionFactor==0) numberTrainingEvents+=1;
      if (i%dilutionFactor==1) numberTestingEvents+=1;

    }
  }
  
  cout << " N. training events: " << numberTrainingEvents << 
    " N. testing events: " << numberTestingEvents << endl;


  cout << "now start to setup the network..." << endl;
  
 
  JetNet* jn = new JetNet( numberTestingEvents, 
			     numberTrainingEvents, 
			     nlayer, 
			     nneurons );

  cout <<  " setting learning method... " << endl;

  //  jn->SetMSTJN(4,12); Fletscher-Rieves (Scaled Conj Grad)

  int nPatternsPerUpdate=200;// || _2 = 200 (before 100) _3,_4=20
  
  jn->SetPatternsPerUpdate( nPatternsPerUpdate );
  jn->SetUpdatesPerEpoch( (int)std::floor((float)numberTrainingEvents/
					  (float)nPatternsPerUpdate) );
  jn->SetUpdatingProcedure( 0 );
  jn->SetErrorMeasure( 0 );
  jn->SetActivationFunction( 1 );
  //  jn->SetLearningRate( 0.5);//0.8 || _2 =0.5 _3=0.05 _4=0.15
  jn->SetLearningRate( 0.5);//0.8 //move to 20 for _3 _4 = 0.15
  //  jn->SetMomentum( 0.3 );//0.3 //is now 0.5 || _2 = 0.3 _3 = 0.03 _4 = 0.05
  jn->SetMomentum( 0.03 );//0.3 //is now 0.5
  jn->SetInitialWeightsWidth( 1. );
  //  jn->SetLearningRateDecrease( 0.992 );
  //  jn->SetLearningRateDecrease( 0.99 );//0.992 || _2 = 0.99 _3 = 0.98 _4=0.99
  jn->SetLearningRateDecrease( 0.99 );//0.992

  std::vector<JetNet::InputNode> jn_input_info; 
  for (std::vector<InputVariableInfo>::const_iterator itr = 
	 input_variables.begin(); 
       itr != input_variables.end(); 
       itr++) { 
    jn_input_info.push_back(convert_node<JetNet::InputNode>(*itr));
  }
  jn->setInputNodes(jn_input_info); 
  
  cout << " copying over training events " << endl;
  
  int training_counter=0;
  for (Int_t i = 0; i < n_entries; i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " Copying over training events. Looping over event " 
		<< i << std::endl;
    }

    if (i%dilutionFactor!=0) continue;
    if (n_training_events_target > 0) { 

      float number_events_remaining = 
	float(n_entries - i) / float(dilutionFactor); 
      float number_events_needed = numberTrainingEvents - training_counter; 
      float keep_event_probibility = 
	number_events_needed / number_events_remaining; 

      float f_rand = float(rand()) / float(RAND_MAX); 
      if (f_rand > keep_event_probibility) continue; 
    }

    simu->GetEntry(i);

    if (bottom==0 && charm==0 && light==0) continue;

    for (int var_num = 0; var_num < in_var.size(); var_num++){ 
      jn->SetInputTrainSet( training_counter, 
			    var_num, 
			    in_var.at(var_num).get_normed() );
    }

    jn->SetOutputTrainSet( training_counter, 0, bottom );
    jn->SetOutputTrainSet( training_counter, 1, charm );
    jn->SetOutputTrainSet( training_counter, 2, light );

    if (fabs(bottom-1) < 1e-4) {
      jn->SetEventWeightTrainSet(  training_counter, weight*bweight );
    }
    else if (fabs(charm-1) < 1e-4){
      jn->SetEventWeightTrainSet(  training_counter, weight*cweight);
    }
    else if (fabs(light-1) < 1e-4) {
      jn->SetEventWeightTrainSet(  training_counter, weight*lweight );
    }
     

    training_counter+=1;

    //not used!
    //    eventWeight=weight;

  }

  if (training_counter != numberTrainingEvents){
    cout << " counter up to: " << training_counter << 
      " while events in training sample are " << 
      numberTrainingEvents << endl;
    return;
  }

  cout << " setting pattern for testing events " << endl;

  
  cout << " copying over testing events " << endl;

  int testing_counter=0;
  for (Int_t i = 0; i < n_entries; i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " Copying over testing events."
	" Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor!=1) continue;

    if (n_training_events_target > 0) { 

      float number_events_remaining = 
	float(n_entries - i) / float(dilutionFactor); 
      float number_events_needed = numberTestingEvents - testing_counter; 
      float keep_event_probibility = 
	number_events_needed / number_events_remaining; 

      float f_rand = float(rand()) / float(RAND_MAX); 
      if (f_rand > keep_event_probibility) continue; 
    }
    
    simu->GetEntry(i);


    if (bottom==0 && charm==0 && light==0) continue;

    for (int var_num = 0; var_num < in_var.size(); var_num++){ 
      jn->SetInputTestSet( testing_counter, 
			   var_num, 
			   in_var.at(var_num).get_normed() );
    }

    jn->SetOutputTestSet( testing_counter, 0, bottom );
    jn->SetOutputTestSet( testing_counter, 1, charm );
    jn->SetOutputTestSet( testing_counter, 2, light );

    if (fabs(bottom-1) < 1e-4) {
	jn->SetEventWeightTestSet( testing_counter, weight*bweight );
      }
    else if (fabs(charm-1) < 1e-4) {
      jn->SetEventWeightTestSet(  testing_counter, weight*cweight );
    }
    else if (fabs(light-1) < 1e-4){
      jn->SetEventWeightTestSet(  testing_counter, weight*lweight );
    }
    testing_counter+=1;

    //not used!
    //    eventWeight=weight;
  }
    
  if (testing_counter != numberTestingEvents){
    cout << " counter up to: " << testing_counter << 
      " while events in testing sample are " << numberTestingEvents << 
      ". Normal due to cuts..." << endl;
    return;  
  }

  //normalize inputvariables?
  //jn->Normalize();

  jn->Shuffle(true,false);
  
  if (restartTrainingFrom == 0) {
    jn->Init();
  }
  else {
    std::stringstream weight_name; 
    weight_name << out_dir; 
    weight_name << "/Weights" << restartTrainingFrom << ".root"; 
    jn->ReadFromFile(weight_name.str().c_str());
  }
  
  float minimumError = 1e10;
  int epochesWithRisingError = 0;
  int epochWithMinimum = 0;

  int updatesPerEpoch = jn->GetUpdatesPerEpoch();

  //prepare output stream
  
  std::string chronology_name = out_dir + "/trainingCronology.txt"; 

  ofstream cronology(chronology_name.c_str(),ios_base::out);
  if (! cronology ) { 
    throw WriteFileException(); 
  }
  
  cronology << "-------------SETTINGS----------------" << endl;
  cronology << "Epochs: " << jn->GetEpochs() << std::endl;
  cronology << "Updates Per Epoch: " << jn->GetUpdatesPerEpoch() << std::endl;
  cronology << "Updating Procedure: " << jn->GetUpdatingProcedure() 
	    << std::endl;
  cronology << "Error Measure: " << jn->GetErrorMeasure() << std::endl;
  cronology << "Patterns Per Update: " << jn->GetPatternsPerUpdate() 
	    << std::endl;
  cronology << "Learning Rate: " << jn->GetLearningRate() << std::endl;
  cronology << "Momentum: " << jn->GetMomentum() << std::endl;
  cronology << "Initial Weights Width: " << jn->GetInitialWeightsWidth() 
	    << std::endl;
  cronology << "Learning Rate Decrease: " << jn->GetLearningRateDecrease() 
	    << std::endl;
  cronology << "Activation Function: " << jn->GetActivationFunction() 
	    << std::endl;
  cronology << "-------------LAYOUT------------------" << endl;
  cronology << "Input variables: " << jn->GetInputDim() << endl;
  cronology << "Output variables: " << jn->GetOutputDim() << endl;
  cronology << "Hidden layers: " << jn->GetHiddenLayerDim() << endl;
  cronology << "Layout : ";
  for (Int_t s=0; s < jn->GetHiddenLayerDim() + 2; ++s) {
    cronology << jn->GetHiddenLayerSize(s);
    if (s < jn->GetHiddenLayerDim()+1) cronology << "-";
  }
  cronology << endl;
  cronology << "--------------HISTORY-----------------" << endl;
  cronology << "History of iterations: " << endl;
  cronology.close();

  //prepare training histo
  TH1F* histoTraining=new TH1F("training","training",
			       (int)std::floor((float)nIterations/10.+0.5),
			       1,std::floor((float)nIterations/10.+1.5));

  TH1F* histoTesting=new TH1F("testing","testing",
			      (int)std::floor((float)nIterations/10.+0.5),
			      1,std::floor((float)nIterations/10.+1.5));

  double maximumTrain=0;
  double minimumTrain=1e10;

  for(int epoch=restartTrainingFrom+1;epoch<=nIterations;++epoch){
    if (epoch!=restartTrainingFrom+1) {
      trainingError = jn->Train();
    }

    if (epoch%10==0 || epoch==restartTrainingFrom+1) {

      cronology.open(chronology_name.c_str(),ios_base::app);

      testError = jn->Test(); 

      if (trainingError > maximumTrain) maximumTrain=trainingError;
      if (testError > maximumTrain) maximumTrain=testError;
      if (trainingError < minimumTrain) minimumTrain=trainingError;
      if (testError < minimumTrain) minimumTrain=testError;

      histoTraining->Fill(epoch/10.0,trainingError);
      histoTesting->Fill(epoch/10.0,testError);

      if (testError<minimumError) {
	minimumError=testError;
	epochesWithRisingError=0;
	epochWithMinimum=epoch;
      }
      else {
	epochesWithRisingError+=10;
	if (trainingError>testError) {
	  epochWithMinimum=epoch;
	}
      }
      
      
      if (epochesWithRisingError>300) {
	if (trainingError<minimumError) {
	  cout << " End of training. Minimum already on epoch: " 
	       << epochWithMinimum << endl;
	  cronology << " End of training. Minimum already on epoch: " 
		    << epochWithMinimum << endl;
	  break;
	} 
      }
      
      cronology << "Epoch: [" << epoch <<
	"] Error: " << trainingError << 
	" Test: " << testError << endl;

      cout << "Epoch: [" << epoch <<
	"] Error: " << trainingError << 
	" Test: " << testError << endl;

      cronology.close();
      
      std::string weight_name = out_dir + "/Weights"; 
      TString name(weight_name.c_str());
      name+=epoch;
      name+=".root";

      TFile* file = new TFile(name,"recreate");
      TTrainedNetwork* trainedNetwork = getTrainedNetwork(*jn);
      file->WriteTObject(trainedNetwork); 


      
      
      // trainedNetwork->Write();
      // file->Write(); //*** SUSPICIOUS: may result in two copies in the file
      
      // --- write in variable tree too 
      // in_var.write_to_file(file); 

      file->Close();
      delete file;

    }
  }
      
  jn->writeNetworkInfo(1);
  jn->writeNetworkInfo(2);
  //  jn->writeNetworkInfo(3);
  //  jn->writeNetworkInfo(4);
  //  jn->writeNetworkInfo(5);

  // ==================================================================
  // ======== end of training (good place to chop in half) ============
  // ==================================================================

  if (epochWithMinimum!=0) {
    cronology << "Minimum stored from Epoch: " << epochWithMinimum << endl;
  }  
  else {
    cronology << "Minimum not reached" << endl;
  }

  cronology.close();

  if (epochWithMinimum != 0) {

    std::string weights_out_name = out_dir + "/Weights"; 
    TString name(weights_out_name.c_str());
    name += epochWithMinimum;
    name += ".root";


    TFile *_file0 = new TFile(name);
    TTrainedNetwork* trainedNetwork=(TTrainedNetwork*)_file0->
      Get("TTrainedNetwork");
    
    // cout << " Reading back network with minimum" << endl;
    // setTrainedNetwork(*jn,trainedNetwork);

    std::string min_weights_name = out_dir + "/weightMinimum.root"; 
    TFile* file=new TFile(min_weights_name.c_str(),"recreate");
    file->WriteTObject(trainedNetwork);

    // write in variable tree too 
    in_var.write_to_file(file); 

    file->Close();
    delete file;


  } 
  else {
    cout << " using network at last iteration (minimum not reached..." << endl;
  }
  
  std::string training_info_name = out_dir + "/trainingInfo.root"; 
  TFile* histoFile=new TFile(training_info_name.c_str(),"recreate");
  histoTraining->Write();
  histoTesting->Write();
  // histoFile->Write();
  histoFile->Close();
  delete histoFile;


}


// ===================================================================
// ========== draw routine (this should be a new function) ===========
// ===================================================================
