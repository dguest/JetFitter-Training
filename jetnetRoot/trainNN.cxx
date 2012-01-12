#include <TTree.h>
#include <TFile.h>
// #include <TCanvas.h>
#include <TH1F.h>
// #include <TLegend.h>
#include <iostream>
// #include <TPad.h>
//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <cmath>
#include "TJetNet.h"
#include "doNormalization.hh"
// #include "Riostream.h"
#include "TNetworkToHistoTool.h"

#include "TTrainedNetwork.h"
#include "trainNN.hh"

#include "OwnerVector.hh"

#include <iostream>

#include "TMatrixD.h"
#include "TVectorD.h"


using namespace std;

int main() 
{

  trainNN("../reduceddatasets/reduceddataset_AntiKt4TopoEMJets_forNN.root", 
	  "dummy",
	  "weights",
	  1000,
	  200,
	  false,
	  false,//withIP3D
	  10,
	  10,
	  0);
  return 0;

}



void trainNN(TString inputfile,
             TString outputclass,
	     std::string out_dir, 
             int nIterations,
             int dilutionFactor,
             bool useSD,
             bool withIP3D,
             int nodesFirstLayer,
             int nodesSecondLayer,
             int restartTrainingFrom) {

  double bweight=1;
  double cweight=1.;
  double lweight=5;

  gROOT->ProcessLine("#include <TTree.h>"); 
  gROOT->ProcessLine("#include <TFile.h>"); 
  
  cout << "starting with settings: " << endl;
  cout << " nIterations: " << nIterations << endl;
  cout << " dilutionFactor: " << dilutionFactor << endl;
  cout << " useSD: " << (useSD==true?"yes":"no") << endl;
  cout << " withIP3D: " << (withIP3D==true?"yes":"no") << endl;
  cout << " nodesFirstLayer: " << nodesFirstLayer << endl;
  cout << " nodesSecondLayer: " << nodesSecondLayer << endl;
  
  
  TFile *file= new TFile(inputfile);
  TTree *simu = (TTree*)file->Get("SVTree");

  Int_t           nVTX;
  Int_t           nTracksAtVtx;
  Int_t           nSingleTracks;
  Double_t        energyFraction;
  Double_t        mass;
  Double_t        significance3d;
  Double_t        discriminatorIP3D;
  Int_t        cat_pT;
  Int_t        cat_eta;
  Double_t        weight;
  Int_t bottom;
  Int_t charm;
  Int_t light;
  
  simu->SetBranchAddress("nVTX",&nVTX);
  simu->SetBranchAddress("nTracksAtVtx",&nTracksAtVtx);
  simu->SetBranchAddress("nSingleTracks",&nSingleTracks);
  simu->SetBranchAddress("energyFraction",&energyFraction);
  simu->SetBranchAddress("mass",&mass);
  simu->SetBranchAddress("significance3d",&significance3d);
  simu->SetBranchAddress("discriminatorIP3D",&discriminatorIP3D);
  simu->SetBranchAddress("cat_pT",&cat_pT);
  simu->SetBranchAddress("cat_eta",&cat_eta);
  simu->SetBranchAddress("weight",&weight);
  simu->SetBranchAddress("bottom",   &bottom);
  simu->SetBranchAddress("charm",   &charm);
  simu->SetBranchAddress("light",&light);

  
  int* nneurons;
  int nlayer=3;

  int numberinputs=8;
  if (withIP3D) {
    numberinputs=9;
  }
  int numberoutputs=3;

  if (nodesSecondLayer!=0){
    nlayer=4;
  }

  if (nodesSecondLayer!=0){
    nneurons=new int[4];
  }
  else {
    nneurons=new int[3];
  }
  
  if (withIP3D) {
    nneurons[0]=9;
  }
  else {
    nneurons[0]=8;
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

  for (Int_t i = 0; i < n_entries; i++) {

    if (i % 100000 == 0 ) {
      std::cout << " Counting training / testing events in sample."
	" Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor==0) numberTrainingEvents+=1;
    //    if (i%dilutionFactor==1||i%dilutionFactor==2) numberTestingEvents+=1;
    if (i%dilutionFactor==1) numberTestingEvents+=1;

  }
  
  cout << " N. training events: " << numberTrainingEvents << 
    " N. testing events: " << numberTestingEvents << endl;

  cout << "now start to setup the network..." << endl;
  
 
  TJetNet* jn = new TJetNet( numberTestingEvents, 
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
  
  cout << " setting pattern for training events " << endl;
  int trainSampleNumber=0;
  int testSampleNumber=1;
  
  cout << " copying over training events " << endl;
  
  int counter=0;
  for (Int_t i = 0; i < n_entries; i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " Copying over training events. Looping over event " << i << std::endl;
    }

    if (i%dilutionFactor!=0) continue;

    simu->GetEntry(i);

    if (bottom==0 && charm==0 && light==0) continue;

    jn->SetInputTrainSet( counter, 0, norm_nVTX(nVTX) );
    jn->SetInputTrainSet( counter, 1, norm_nTracksAtVtx(nTracksAtVtx) );
    jn->SetInputTrainSet( counter, 2, norm_nSingleTracks(nSingleTracks) );
    jn->SetInputTrainSet( counter, 3, norm_energyFraction(energyFraction) );
    jn->SetInputTrainSet( counter, 4, norm_mass(mass) );
    jn->SetInputTrainSet( counter, 5, norm_significance3d(significance3d ) );
    if (withIP3D){
      jn->SetInputTrainSet( counter, 6, norm_IP3D(discriminatorIP3D) );
      jn->SetInputTrainSet( counter, 7, norm_cat_pT(cat_pT) );
      jn->SetInputTrainSet( counter, 8, norm_cat_eta(cat_eta) );
    }
    else {
      jn->SetInputTrainSet( counter, 6, norm_cat_pT(cat_pT) );
      jn->SetInputTrainSet( counter, 7, norm_cat_eta(cat_eta) );
    }

    jn->SetOutputTrainSet( counter, 0, bottom );
    jn->SetOutputTrainSet( counter, 1, charm );
    jn->SetOutputTrainSet( counter, 2, light );

    if (fabs(bottom-1) < 1e-4) {
      jn->SetEventWeightTrainSet(  counter, weight*bweight );
    }
    else if (fabs(charm-1) < 1e-4){
      jn->SetEventWeightTrainSet(  counter, weight*cweight);
    }
    else if (fabs(light-1) < 1e-4) {
      jn->SetEventWeightTrainSet(  counter, weight*lweight );
    }
     

    counter+=1;

    //not used!
    //    eventWeight=weight;

  }

  if (counter!=numberTrainingEvents){
    cout << " counter up to: " << counter << 
      " while events in training sample are " << 
      numberTrainingEvents << endl;
    return;
  }

  cout << " setting pattern for testing events " << endl;

  
  cout << " copying over testing events " << endl;
  counter=0;
  
  
  for (Int_t i = 0; i < n_entries; i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " Copying over testing events."
	" Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor!=1) continue;
    
    simu->GetEntry(i);


    if (bottom==0 && charm==0 && light==0) continue;

    jn->SetInputTestSet( counter, 0, norm_nVTX(nVTX) );
    jn->SetInputTestSet( counter, 1, norm_nTracksAtVtx(nTracksAtVtx) );
    jn->SetInputTestSet( counter, 2, norm_nSingleTracks(nSingleTracks) );
    jn->SetInputTestSet( counter, 3, norm_energyFraction(energyFraction) );
    jn->SetInputTestSet( counter, 4, norm_mass(mass) );
    jn->SetInputTestSet( counter, 5, norm_significance3d(significance3d ) );
    if (withIP3D){
	jn->SetInputTestSet( counter, 6, norm_IP3D(discriminatorIP3D) );
	jn->SetInputTestSet( counter, 7, norm_cat_pT(cat_pT) );
	jn->SetInputTestSet( counter, 8, norm_cat_eta(cat_eta) );
    }
    else {
      jn->SetInputTestSet( counter, 6, norm_cat_pT(cat_pT) );
      jn->SetInputTestSet( counter, 7, norm_cat_eta(cat_eta) );
    }

    jn->SetOutputTestSet( counter, 0, bottom );
    jn->SetOutputTestSet( counter, 1, charm );
    jn->SetOutputTestSet( counter, 2, light );

    if (fabs(bottom-1) < 1e-4) {
	jn->SetEventWeightTestSet(  counter, weight*bweight );
      }
    else if (fabs(charm-1) < 1e-4) {
      jn->SetEventWeightTestSet(  counter, weight*cweight );
    }
    else if (fabs(light-1) < 1e-4){
      jn->SetEventWeightTestSet(  counter, weight*lweight );
    }
    counter+=1;

    //not used!
    //    eventWeight=weight;
  }
    
  if (counter!=numberTestingEvents){
    cout << " counter up to: " << counter << 
      " while events in testing sample are " << numberTestingEvents << 
      ". Normal due to cuts..." << endl;
    return;  
  }

  //normalize inputvariables?
  //jn->Normalize();

  jn->Shuffle(true,false);
  
  if (restartTrainingFrom==0) {
    jn->Init();
    //    jn->DumpToFile("WeightsInitial.w");
  }
  else {
    TString name("Weights");
    name+=restartTrainingFrom;
    name+=".w";

    jn->ReadFromFile(name);
  }
  
  float minimumError = 1e10;
  int epochesWithRisingError = 0;
  int epochWithMinimum = 0;

  int updatesPerEpoch = jn->GetUpdatesPerEpoch();

  //prepare output stream
  
  std::string chronology_name = out_dir + "/trainingCronology.txt"; 

  ofstream cronology(chronology_name.c_str(),ios_base::out);//|ios_base::app);
  
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

      testError = jn->TestBTAG();

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

      TFile* file=new TFile(name,"recreate");
      TTrainedNetwork* trainedNetwork=jn->createTrainedNetwork();
      trainedNetwork->Write();
      file->Write(); //*** SUSPICIOUS: may result in two copies in the file
      file->Close();
      delete file;

      /*
	TFile* file2=new TFile(name);
	trainedNetwork=(TTrainedNetwork*)file2->Get("TTrainedNetwork");
	cout <<" hid lay 1 size: " << trainedNetwork->getnHiddenLayerSize()[0] << endl;
	file2->Close();
	delete file2;
      */

      //      jn->DumpToFile(name);
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

  Int_t nInput=jn->GetInputDim();
  
  cout << " create Trained Network object..." << endl;
  
  TTrainedNetwork* trainedNetwork = jn->createTrainedNetwork();

  cout << " now getting value with trained Network ";

  double inputexample[9] = {norm_nVTX(1),
			    norm_nTracksAtVtx(2),
			    norm_nSingleTracks(0),
			    norm_energyFraction(0.6),
			    norm_mass(2500),
			    norm_significance3d(4 ),
			    norm_IP3D(3),
			    norm_cat_pT(3),
			    norm_cat_eta(1)};

  for (Int_t i=0;i<nInput;++i){
    jn->SetInputs(i,inputexample[i]);
  }

  
  cronology.open(chronology_name.c_str(), ios_base::app);

  jn->Evaluate();

  cronology << "----------------CONSISTENCY CHECK-----------" << endl;
  cout << "Result 0:" << jn->GetOutput(0);
  cronology << "Result 0:" << jn->GetOutput(0);
  cout << " Result 1:" << jn->GetOutput(1);
  cronology << "Result 0:" << jn->GetOutput(1);
  cout << " Result 2:" << jn->GetOutput(2) << endl;
  cronology << " Result 2:" << jn->GetOutput(2) << endl;

  cout << " Reading back old network " << endl;
  jn->readBackTrainedNetwork(trainedNetwork);

  cout <<" resetting input " << endl;
  for (Int_t i=0;i<nInput;++i) {
    jn->SetInputs(i,inputexample[i]);
  }

  jn->Evaluate();
 
  cout << "After reading back - Result 0:" << jn->GetOutput(0);
  cronology << "After reading back - Result 0:" << jn->GetOutput(0);
  // <<     " my: " << result[0] << endl;
  cout << " After reading back - Result 1:" << jn->GetOutput(1);
  cronology << "After reading back - Result 1:" << jn->GetOutput(1);
  //<<     " my: " << result[1] << endl;
  cout << " After reading back - Result 2:" << jn->GetOutput(2) << endl;
  cronology << "After reading back - Result 2:" << jn->GetOutput(2);
  // << " my: " << result[2] << endl;

  cout << " Now getting histograms from trainingResult" << endl;
  cronology << " Now getting histograms from trainingResult" << endl;

  TNetworkToHistoTool myHistoTool;

  cout << " From network to histo..." << endl;
  OwnerVector<TH1*> myHistos = myHistoTool.
    fromTrainedNetworkToHisto(trainedNetwork);

  cout << " From histo to network back..." << endl;
  TTrainedNetwork* trainedNetwork2 = myHistoTool.
    fromHistoToTrainedNetwork(myHistos);

  cout << " reading back " << endl;
  jn->readBackTrainedNetwork(trainedNetwork2);
   
  cout <<" resetting input " << endl;
  for (Int_t i=0;i<nInput;++i) {
    jn->SetInputs(i,inputexample[i]);
  }

  jn->Evaluate();

  cout << "After reading back - Result 0:" << jn->GetOutput(0);
  cronology << "After reading back - Result 0:" << jn->GetOutput(0);
  // <<     " my: " << result[0] << endl;
  cout << " After reading back - Result 1:" << jn->GetOutput(1);
  cronology << "After reading back - Result 1:" << jn->GetOutput(1);
  //<<     " my: " << result[1] << endl;
  cout << " After reading back - Result 2:" << jn->GetOutput(2) << endl;
  cronology << "After reading back - Result 2:" << jn->GetOutput(2);
  // << " my: " << result[2] << endl;
  
  cout << " Directly from the trainedNetwork read back from HISTOS...!" << endl;

  std::vector<Double_t> inputData;
  for (Int_t u=0;u<nInput;++u) {
    inputData.push_back(inputexample[u]);
  }

  std::vector<Double_t> outputData=trainedNetwork2->
    calculateOutputValues(inputData);

  cout << "After reading back - Result 0:" << outputData[0] << endl;
  cout << " After reading back - Result 1:" << outputData[1] << endl;
  cout << " After reading back - Result 2:" << outputData[2] << endl;
   

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

    std::cout << " reading back from minimum " << endl;


    TFile *_file0 = new TFile(name);
    TTrainedNetwork* trainedNetwork=(TTrainedNetwork*)_file0->
      Get("TTrainedNetwork");
    
    cout << " Reading back network with minimum" << endl;
    jn->readBackTrainedNetwork(trainedNetwork);

    std::string min_weights_name = out_dir + "/weightMinimum.root"; 
    TFile* file=new TFile(min_weights_name.c_str(),"recreate");
    trainedNetwork->Write();
    file->Write();
    file->Close();
    delete file;

    cout << " -------------------- " << endl;
    cout << " Writing OUTPUT histos " << endl;
    std::string histo_weights_name = out_dir + "/histoWeights.root"; 
    TFile* fileHistos=new TFile(histo_weights_name.c_str(),"recreate");
    TNetworkToHistoTool histoTool;
    std::vector<TH1*> myHistos=histoTool.
      fromTrainedNetworkToHisto(trainedNetwork);

    for (std::vector<TH1*>::iterator histoIter = myHistos.begin();
	 histoIter != myHistos.end(); 
	 ++histoIter) {
      (*histoIter)->Write();
      delete *histoIter; 
      *histoIter = 0; 
    }

    // SUSPICIOUS: may be writing twice here too
    // fileHistos->Write();
    fileHistos->Close();
    delete fileHistos;

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

  // TCanvas* trainingCanvas=new TCanvas("trainingCanvas","trainingCanvas");
  // histoTraining->SetLineColor(2);
  // histoTesting->SetLineColor(4);


  // histoTraining->GetYaxis()->SetRangeUser(minimumTrain,maximumTrain);
  // histoTraining->Draw("l");
  // histoTesting->Draw("lsame");
  // std::string training_curve_name = out_dir + "/trainingCurve.eps"; 
  // trainingCanvas->SaveAs(training_curve_name.c_str());



}

// ===================================================================
// ========== draw routine (this should be a new function) ===========
// ===================================================================
