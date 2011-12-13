#include <Python.h>
#include <TTree.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TLegend.h>
#include <iostream>
#include <TPad.h>
//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include <math.h>
#include "TJetNet.h"
#include "doNormalization.C"
#include "Riostream.h"
#include "TNetworkToHistoTool.h"

#include "TTrainedNetwork.h"

#include <iostream>

#include "TMatrixD.h"
#include "TVectorD.h"


using namespace std;

Double_t sigmoid(Double_t x)
{
  return 1./(1.+exp(-2*x));
}



using namespace std;

void trainNN(TString inputfile,
             TString outputclass="JetFitterNN",
             int nIterations=10,
             int dilutionFactor=2,
             bool useSD=false,
             bool withIP3D=true,
             int nodesFirstLayer=10,
             int nodesSecondLayer=9,
             int restartTrainingFrom=0);

static char* doc_string = "run the nn\n"; 

static PyObject* train_py(PyObject *self, 
			  PyObject *args, 
			  PyObject *keywds)
{
  int voltage;
  char *state = "a stiff";
  char *action = "voom";
  char *type = "Norwegian Blue";

  char* input_file; 
  char* output_class = "JetFitterNN"; 
  int n_iterations = 10; 
  int dilution_factor = 2; 
  bool use_sd = false; 
  bool with_ip3d = true; 
  int nodes_first_layer = 10; 
  int nodes_second_layer = 9; 
  int restart_training_from = 0; 
  bool debug = false; 

  static char *kwlist[] = {"input_file", "output_class", "n_iterations", 
			   "dilution_factor","use_sd","with_ip3d",
			   "nodes_first_layer","nodes_second_layer", 
			   "restart_training_from", "debug", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|siibbiiib", kwlist,
				   &input_file, &output_class, 
				   &n_iterations, &dilution_factor, 
				   &use_sd, &with_ip3d, &nodes_first_layer, 
				   &nodes_second_layer, 
				   &restart_training_from, 
				   &debug))
    return NULL;

  if (debug){ 
    printf("in = %s, out = %s, itr = %i, rest from = %i\n", 
  	   input_file, output_class, n_iterations, restart_training_from); 
  }

  else{ 
    trainNN(TString(input_file), 
	    TString(output_class),
	    n_iterations,
	    dilution_factor,
	    use_sd,
	    with_ip3d,
	    nodes_first_layer,
	    nodes_second_layer,
	    restart_training_from);
  }

  Py_INCREF(Py_None);

  return Py_None;
}

const char* getDocString(const char* kw_list[], int n_entries){ 
  std::string doc = "run the neural net, inputs:\n"; 
  for (int entry_n = 0; entry_n < n_entries; entry_n++){ 
    doc.append(kw_list[entry_n]);
    doc.append("\n"); 
  }
  return doc.c_str(); 
}


static PyMethodDef keywdarg_methods[] = {
    /* The cast of the function is necessary since PyCFunction values
     * only take two PyObject* parameters, and keywdarg_parrot() takes
     * three.
     */
  {"trainNN", (PyCFunction)train_py, 
   METH_VARARGS | METH_KEYWORDS,
   doc_string},
  // {"parrot", (PyCFunction)keywdarg_parrot,
  //  METH_VARARGS | METH_KEYWORDS,
  //  "Print a lovely skit to standard output."},
  // {"system",  spam_system, METH_VARARGS,
  //  "Execute a shell command."},

    {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" PyMODINIT_FUNC initpynn(void)
{
  Py_InitModule("pynn", keywdarg_methods);
}


int doIt()
{
  trainNN(
      "../reduceddatasets/reduceddataset_Cone4H1TopoParticleJets_forNN.root",
      "dummy",
      10000,
      200,
      false,
      false,//withIP3D
      10,
      10,
      0);
  return 0;
}


int main() 
{

  trainNN("../reduceddatasets/reduceddataset_AntiKt4TopoEMJets_forNN.root", 
	  "dummy",
	  10000,
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

  gROOT->SetStyle("Plain");

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


  TString filterTrain("Entry$%");
  filterTrain+=dilutionFactor;
  filterTrain+="==0";
  
  TString filterTest("Entry$%");
  filterTest+=dilutionFactor;
  filterTest+="==1";

  int* nneurons;
  int nlayer=3;

  int numberinputs=8;
  if (withIP3D)
  {
    numberinputs=9;
  }
  int numberoutputs=3;

  if (nodesSecondLayer!=0)
  {
    nlayer=4;
  }

  if (nodesSecondLayer!=0)
  {
    nneurons=new int[4];
  }
  else
  {
    nneurons=new int[3];
  }
  
  if (withIP3D)
  {
    nneurons[0]=9;
  }
  else
  {
    nneurons[0]=8;
  }
  
  nneurons[1]=nodesFirstLayer;

  if (nodesSecondLayer!=0)
  {
    nneurons[2]=nodesSecondLayer;
    nneurons[3]=3;
  }
  else
  {
    nneurons[2]=3;
  }

  //  float eventWeight(0);
  float trainingError(0);
  float testError(0);
  
  //setting learning parameters

  cout << " now providing training events " << endl;
  
  Int_t numberTrainingEvents=0;
  Int_t numberTestingEvents=0;

  for (Int_t i = 0; i < simu->GetEntries(); i++) {

    if (i % 100000 == 0 ) {
      std::cout << " Counting training / testing events in sample. Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor==0) numberTrainingEvents+=1;
//    if (i%dilutionFactor==1||i%dilutionFactor==2) numberTestingEvents+=1;
    if (i%dilutionFactor==1) numberTestingEvents+=1;

  }
  
  cout << " N. training events: " << numberTrainingEvents << 
      " N. testing events: " << numberTestingEvents << endl;

  cout << "now start to setup the network..." << endl;
  
 
  TJetNet* jn = new TJetNet( numberTestingEvents, numberTrainingEvents, nlayer, nneurons );

  cout <<  " setting learning method... " << endl;

  //  jn->SetMSTJN(4,12); Fletscher-Rieves (Scaled Conj Grad)

  int nPatternsPerUpdate=200;// || _2 = 200 (before 100) _3,_4=20
  
  jn->SetPatternsPerUpdate( nPatternsPerUpdate );
  jn->SetUpdatesPerEpoch( (int)std::floor((float)numberTrainingEvents/(float)nPatternsPerUpdate) );
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
  for (Int_t i = 0; i < simu->GetEntries(); i++) {
    
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
    if (withIP3D)
    {
      jn->SetInputTrainSet( counter, 6, norm_IP3D(discriminatorIP3D) );
      jn->SetInputTrainSet( counter, 7, norm_cat_pT(cat_pT) );
      jn->SetInputTrainSet( counter, 8, norm_cat_eta(cat_eta) );
    }
    else
    {
      jn->SetInputTrainSet( counter, 6, norm_cat_pT(cat_pT) );
      jn->SetInputTrainSet( counter, 7, norm_cat_eta(cat_eta) );
    }

    jn->SetOutputTrainSet( counter, 0, bottom );
    jn->SetOutputTrainSet( counter, 1, charm );
    jn->SetOutputTrainSet( counter, 2, light );

    if (fabs(bottom-1)<1e-4)
    {
      jn->SetEventWeightTrainSet(  counter, weight*bweight );
    }
    else if (fabs(charm-1)<1e-4)
    {
      jn->SetEventWeightTrainSet(  counter, weight*cweight);
    }
    else if (fabs(light-1)<1e-4)
    {
      jn->SetEventWeightTrainSet(  counter, weight*lweight );
    }
     

    counter+=1;

    //not used!
    //    eventWeight=weight;

  }

  if (counter!=numberTrainingEvents)
  {
    cout << " counter up to: " << counter << " while events in training sample are " << numberTrainingEvents << endl;
    return;
  }

  cout << " setting pattern for testing events " << endl;

  
  cout << " copying over testing events " << endl;
  counter=0;
  
  
  for (Int_t i = 0; i < simu->GetEntries(); i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " Copying over testing events. Looping over event " << i << std::endl;
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
    if (withIP3D)
    {
      jn->SetInputTestSet( counter, 6, norm_IP3D(discriminatorIP3D) );
      jn->SetInputTestSet( counter, 7, norm_cat_pT(cat_pT) );
      jn->SetInputTestSet( counter, 8, norm_cat_eta(cat_eta) );
    }
    else
    {
      jn->SetInputTestSet( counter, 6, norm_cat_pT(cat_pT) );
      jn->SetInputTestSet( counter, 7, norm_cat_eta(cat_eta) );
    }

    jn->SetOutputTestSet( counter, 0, bottom );
    jn->SetOutputTestSet( counter, 1, charm );
    jn->SetOutputTestSet( counter, 2, light );

    if (fabs(bottom-1)<1e-4)
    {
      jn->SetEventWeightTestSet(  counter, weight*bweight );
    }
    else if (fabs(charm-1)<1e-4)
    {
      jn->SetEventWeightTestSet(  counter, weight*cweight );
    }
    else if (fabs(light-1)<1e-4)
    {
      jn->SetEventWeightTestSet(  counter, weight*lweight );
    }
    counter+=1;

    //not used!
    //    eventWeight=weight;
  }
    
  if (counter!=numberTestingEvents)
  {
 cout << " counter up to: " << counter << " while events in testing sample are " << numberTestingEvents << ". Normal due to cuts..." << endl;
    return;  }

  //normalize inputvariables?
  //jn->Normalize();

  jn->Shuffle(true,false);
  
  if (restartTrainingFrom==0)
  {
    jn->Init();
    //    jn->DumpToFile("WeightsInitial.w");
  }
  else
  {
    TString name("Weights");
    name+=restartTrainingFrom;
    name+=".w";

    jn->ReadFromFile(name);
  }
  


  float minimumError=1e10;
  int epochesWithRisingError=0;
  int epochWithMinimum=0;

  int updatesPerEpoch=jn->GetUpdatesPerEpoch();

  //prepare output stream

  ofstream cronology("weights/trainingCronology.txt",ios_base::out);//|ios_base::app);
  
  cronology << "-------------SETTINGS----------------" << endl;
  cronology << "Epochs: " << jn->GetEpochs() << std::endl;
  cronology << "Updates Per Epoch: " << jn->GetUpdatesPerEpoch() << std::endl;
  cronology << "Updating Procedure: " << jn->GetUpdatingProcedure() << std::endl;
  cronology << "Error Measure: " << jn->GetErrorMeasure() << std::endl;
  cronology << "Patterns Per Update: " << jn->GetPatternsPerUpdate() << std::endl;
  cronology << "Learning Rate: " << jn->GetLearningRate() << std::endl;
  cronology << "Momentum: " << jn->GetMomentum() << std::endl;
  cronology << "Initial Weights Width: " << jn->GetInitialWeightsWidth() << std::endl;
  cronology << "Learning Rate Decrease: " << jn->GetLearningRateDecrease() << std::endl;
  cronology << "Activation Function: " << jn->GetActivationFunction() << std::endl;
  cronology << "-------------LAYOUT------------------" << endl;
  cronology << "Input variables: " << jn->GetInputDim() << endl;
  cronology << "Output variables: " << jn->GetOutputDim() << endl;
  cronology << "Hidden layers: " << jn->GetHiddenLayerDim() << endl;
  cronology << "Layout : ";
  for (Int_t s=0;s<jn->GetHiddenLayerDim()+2;++s)
  {
    cronology << jn->GetHiddenLayerSize(s);
    if (s<jn->GetHiddenLayerDim()+1) cronology << "-";
  }
  cronology << endl;
  cronology << "--------------HISTORY-----------------" << endl;
  cronology << "History of iterations: " << endl;
  cronology.close();

  //prepare training histo
  TH1F* histoTraining=new TH1F("training","training",(int)std::floor((float)nIterations/10.+0.5),1,std::floor((float)nIterations/10.+1.5));
  TH1F* histoTesting=new TH1F("testing","testing",(int)std::floor((float)nIterations/10.+0.5),1,std::floor((float)nIterations/10.+1.5));

  double maximumTrain=0;
  double minimumTrain=1e10;

  for(int epoch=restartTrainingFrom+1;epoch<=nIterations;++epoch)
  {
    if (epoch!=restartTrainingFrom+1)
    {
      trainingError = jn->Train();
    }

    if (epoch%10==0 || epoch==restartTrainingFrom+1)
    {

      cronology.open("weights/trainingCronology.txt",ios_base::app);

      testError = jn->TestBTAG();

      if (trainingError>maximumTrain) maximumTrain=trainingError;
      if (testError>maximumTrain) maximumTrain=testError;
      if (trainingError<minimumTrain) minimumTrain=trainingError;
      if (testError<minimumTrain) minimumTrain=testError;

      
      histoTraining->Fill(epoch/10.,trainingError);
      histoTesting->Fill(epoch/10.,testError);

      if (testError<minimumError)
      {
        minimumError=testError;
        epochesWithRisingError=0;
        epochWithMinimum=epoch;
      }
      else
      {
        epochesWithRisingError+=10;
        if (trainingError>testError)
        {
          epochWithMinimum=epoch;
        }
      }
      
      
      if (epochesWithRisingError>300)
      {
	if (trainingError<minimumError)
	{
	  cout << " End of training. Minimum already on epoch: " << epochWithMinimum << endl;
          cronology << " End of training. Minimum already on epoch: " << epochWithMinimum << endl;
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
      
      TString name("weights/Weights");
      name+=epoch;
      name+=".root";

      TFile* file=new TFile(name,"recreate");
      TTrainedNetwork* trainedNetwork=jn->createTrainedNetwork();
      trainedNetwork->Write();
      file->Write();
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


  //  cout << " Now try to understand how to get the weights..." << endl;

  ////////////WWWWWAAAAASSSSSS HERE
  Int_t nInput=jn->GetInputDim();
  
  cout << " create Trained Network object..." << endl;
  
  TTrainedNetwork* trainedNetwork=jn->createTrainedNetwork();

  cout << " now getting value with trained Network ";

  


  double inputexample[9]={norm_nVTX(1),
			  norm_nTracksAtVtx(2),
			  norm_nSingleTracks(0),
			  norm_energyFraction(0.6),
			  norm_mass(2500),
			  norm_significance3d(4 ),
			  norm_IP3D(3),
			  norm_cat_pT(3),
			  norm_cat_eta(1)};

  for (Int_t i=0;i<nInput;++i)
  {
    jn->SetInputs(i,inputexample[i]);
  }

  cronology.open("weights/trainingCronology.txt",ios_base::app);

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
  for (Int_t i=0;i<nInput;++i)
  {
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
  std::vector<TH1*> myHistos=myHistoTool.fromTrainedNetworkToHisto(trainedNetwork);

  cout << " From histo to network back..." << endl;
  TTrainedNetwork* trainedNetwork2=myHistoTool.fromHistoToTrainedNetwork(myHistos);
  
  cout << " reading back " << endl;
  jn->readBackTrainedNetwork(trainedNetwork2);
   
  cout <<" resetting input " << endl;
  for (Int_t i=0;i<nInput;++i)
  {
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
  for (Int_t u=0;u<nInput;++u)
  {
    inputData.push_back(inputexample[u]);
  }

  std::vector<Double_t> outputData=trainedNetwork2->calculateOutputValues(inputData);

  cout << "After reading back - Result 0:" << outputData[0] << endl;
  cout << " After reading back - Result 1:" << outputData[1] << endl;
  cout << " After reading back - Result 2:" << outputData[2] << endl;
   

  


  if (epochWithMinimum!=0)
  {
    cronology << "Minimum stored from Epoch: " << epochWithMinimum << endl;
  }  else
  {
    cronology << "Minimum not reached" << endl;
  }

  cronology.close();

  if (epochWithMinimum!=0)
  {
    
      TString name("weights/Weights");
      name+=epochWithMinimum;
      name+=".root";

      std::cout << " reading back from minimum " << endl;


      TFile *_file0 = new TFile(name);
      TTrainedNetwork* trainedNetwork=(TTrainedNetwork*)_file0->Get("TTrainedNetwork");

      cout << " Reading back network with minimum" << endl;
      jn->readBackTrainedNetwork(trainedNetwork);

      TFile* file=new TFile("weights/weightMinimum.root","recreate");
      trainedNetwork->Write();
      file->Write();
      file->Close();
      delete file;

      cout << " -------------------- " << endl;
      cout << " Writing OUTPUT histos " << endl;
      TFile* fileHistos=new TFile("weights/histoWeights.root","recreate");
      TNetworkToHistoTool histoTool;
      std::vector<TH1*> myHistos=histoTool.fromTrainedNetworkToHisto(trainedNetwork);
      std::vector<TH1*>::const_iterator histoBegin=myHistos.begin();
      std::vector<TH1*>::const_iterator histoEnd=myHistos.end();
      for (std::vector<TH1*>::const_iterator histoIter=histoBegin;
           histoIter!=histoEnd;++histoIter)
      {
        (*histoIter)->Write();
      }
      fileHistos->Write();
      fileHistos->Close();
      delete fileHistos;

      //        " filename: " << name << endl;
      
    //    jn->ReadFromFile(name);

  } 
  else
  {
    cout << " using network at last iteration (minimum not reached..." << endl;
  }
  
  //here you should create the class... Still open how to deal with this...
  //  char* myname=const_cast<char*>(static_cast<const char*>(outputclass));
  //  ierr=mlpsavecf_(myname);
 
  TFile* histoFile=new TFile("weights/trainingInfo.root","recreate");
  histoTraining->Write();
  histoTesting->Write();
  histoFile->Write();
  histoFile->Close();
  delete histoFile;

  TCanvas* trainingCanvas=new TCanvas("trainingCanvas","trainingCanvas");
  histoTraining->SetLineColor(2);
  histoTesting->SetLineColor(4);

  histoTraining->GetYaxis()->SetRangeUser(minimumTrain,maximumTrain);
  histoTraining->Draw("l");
  histoTesting->Draw("lsame");
  trainingCanvas->SaveAs("weights/trainingCurve.eps");

 
  TCanvas* mlpa_canvas = new TCanvas("jetnet_canvas","Network analysis");
  mlpa_canvas->Divide(2,4);


  
//  TCanvas* mlpa_canvas_5=gDirectory->Get("mlpa_canvas_5");
//  mlpa_canvas_5->SetLogy(kTrue);
  gPad->SetLogy();

  // Use the NN to plot the results for each sample
  // This will give approx. the same result as DrawNetwork.
  // All entries are used, while DrawNetwork focuses on 
  // the test sample. Also the xaxis range is manually set.
  TH1F *bg2 = new TH1F("bg2h", "NN output", 50, -.5, 1.5);
  TH1F *bg = new TH1F("bgh", "NN output", 50, -.5, 1.5);
  TH1F *sig = new TH1F("sigh", "NN output", 50, -.5, 1.5);

  TH1F *bg2test = new TH1F("bg2htest", "NN output", 50, -.5, 1.5);
  TH1F *bgtest = new TH1F("bghtest", "NN output", 50, -.5, 1.5);
  TH1F *sigtest = new TH1F("sightest", "NN output", 50, -.5, 1.5);



      
  for (Int_t i = 0; i < simu->GetEntries(); i++) {
    
    if (i % 100000 == 0 ) {
      std::cout << " First plot. Looping over event " << i << std::endl;
    }
    
    if (i%dilutionFactor!=0&&i%dilutionFactor!=1) continue;
    
    simu->GetEntry(i);

    jn->SetInputs(0, norm_nVTX(nVTX) );
    jn->SetInputs(1, norm_nTracksAtVtx(nTracksAtVtx) );
    jn->SetInputs(2, norm_nSingleTracks(nSingleTracks) );
    jn->SetInputs(3, norm_energyFraction(energyFraction) );
    jn->SetInputs(4, norm_mass(mass) );
    jn->SetInputs(5, norm_significance3d(significance3d ) );
    if (withIP3D)
    {
      jn->SetInputs(6, norm_IP3D(discriminatorIP3D) );
      jn->SetInputs(7, norm_cat_pT(cat_pT) );
      jn->SetInputs(8, norm_cat_eta(cat_eta) );
    }
    else
    {
      jn->SetInputs(6, norm_cat_pT(cat_pT) );
      jn->SetInputs(7, norm_cat_eta(cat_eta) );
    }

    jn->Evaluate();

    float bvalue=jn->GetOutput(0);
    float lvalue=jn->GetOutput(2);



    if (bottom==1)
    {
      if (i%dilutionFactor==0)
      {
        sig->Fill(bvalue/(bvalue+lvalue),weight);
      }
      else if (i%dilutionFactor==1)
      {
        sigtest->Fill(bvalue/(bvalue+lvalue),weight);
      }
    }
    if (light==1)
    {
      if (i%dilutionFactor==0)
      {
        bg->Fill(bvalue/(bvalue+lvalue),weight);
      }
      else if (i%dilutionFactor==1)
      {
        bgtest->Fill(bvalue/(bvalue+lvalue),weight);
      }
    }
    if (charm==1)
    {
      if (i%dilutionFactor==0)
      {
        bg2->Fill(bvalue/(bvalue+lvalue),weight);
      }
      else  if (i%dilutionFactor==1)
      {
        bg2test->Fill(bvalue/(bvalue+lvalue),weight);
      }
    }
  }

  //now you need the maximum
  float maximum=1;
  for (Int_t a=0;a<bg->GetNbinsX();a++)
  {
    if (bg->GetBinContent(a)>maximum)
    {
      maximum=1.2*bg->GetBinContent(a);
    }
  }


  bg2->SetLineColor(kYellow);
  bg2->SetFillStyle(3008);   bg2->SetFillColor(kYellow);
  bg->SetLineColor(kBlue);
  bg->SetFillStyle(3008);   bg->SetFillColor(kBlue);
  sig->SetLineColor(kRed);
  sig->SetFillStyle(3003); sig->SetFillColor(kRed);
  bg2->SetStats(0);
  bg->SetStats(0);
  sig->SetStats(0);


  bg2test->SetLineColor(kYellow);
  bg2test->SetFillStyle(3008);   bg2test->SetFillColor(kYellow);
  bgtest->SetLineColor(kBlue);
  bgtest->SetFillStyle(3008);   bgtest->SetFillColor(kBlue);
  sigtest->SetLineColor(kRed);
  sigtest->SetFillStyle(3003); sigtest->SetFillColor(kRed);
  bg2test->SetStats(0);
  bgtest->SetStats(0);
  sigtest->SetStats(0);

 mlpa_canvas->cd(1);
 gPad->SetLogy();

 bg->GetYaxis()->SetRangeUser(1,maximum);
 bgtest->GetYaxis()->SetRangeUser(1,maximum);

 mlpa_canvas->cd(1);
 bg->Draw();
 bg2->Draw("same");
 sig->Draw("same");

 TLegend *legend = new TLegend(.75, .80, .95, .95);
 legend->AddEntry(bg2, "Background2 (charm)");
 legend->AddEntry(bg, "Background (light)");
 legend->AddEntry(sig, "Signal (bottom)");
 legend->Draw();
 
 mlpa_canvas->cd(2);
 gPad->SetLogy();

 bgtest->Draw();
 bg2test->Draw("same");
 sigtest->Draw("same");

 TLegend *legendtest = new TLegend(.75, .80, .95, .95);
 legendtest->AddEntry(bg2test, "Background2 (charm)");
 legendtest->AddEntry(bgtest, "Background (light)");
 legendtest->AddEntry(sigtest, "Signal (bottom)");
 legendtest->Draw();

 mlpa_canvas->cd(5);
 gPad->SetLogy();
 bg->DrawNormalized();
 bg2->DrawNormalized("same");
 sig->DrawNormalized("same");
 legend->Draw();
 
 mlpa_canvas->cd(6);
 gPad->SetLogy();
 bgtest->DrawNormalized();
 bg2test->DrawNormalized("same");
 sigtest->DrawNormalized("same");
 legendtest->Draw();


 
 mlpa_canvas->cd(3);
 gPad->SetLogy();
 
 // Use the NN to plot the results for each sample
 // This will give approx. the same result as DrawNetwork.
 // All entries are used, while DrawNetwork focuses on 
 // the test sample. Also the xaxis range is manually set.
 TH1F *c_bg2 = new TH1F("c_bg2h", "NN output", 50, -.5, 1.5);
 TH1F *c_bg = new TH1F("c_bgh", "NN output", 50, -.5, 1.5);
 TH1F *c_sig = new TH1F("c_sigh", "NN output", 50, -.5, 1.5);

 TH1F *c_bg2test = new TH1F("c_bg2htest", "NN output", 50, -.5, 1.5);
 TH1F *c_bgtest = new TH1F("c_bghtest", "NN output", 50, -.5, 1.5);
 TH1F *c_sigtest = new TH1F("c_sightest", "NN output", 50, -.5, 1.5);

 for (Int_t i = 0; i < simu->GetEntries(); i++) {
   
   if (i % 100000 == 0 ) {
     std::cout << " Second plot. Looping over event " << i << std::endl;
   }
   
   if (i%dilutionFactor!=0&&i%dilutionFactor!=1) continue;
   
   simu->GetEntry(i);

    jn->SetInputs(0, norm_nVTX(nVTX) );
    jn->SetInputs(1, norm_nTracksAtVtx(nTracksAtVtx) );
    jn->SetInputs(2, norm_nSingleTracks(nSingleTracks) );
    jn->SetInputs(3, norm_energyFraction(energyFraction) );
    jn->SetInputs(4, norm_mass(mass) );
    jn->SetInputs(5, norm_significance3d(significance3d ) );
    if (withIP3D)
    {
      jn->SetInputs(6, norm_IP3D(discriminatorIP3D) );
      jn->SetInputs(7, norm_cat_pT(cat_pT) );
      jn->SetInputs(8, norm_cat_eta(cat_eta) );
    }
    else
    {
      jn->SetInputs(6, norm_cat_pT(cat_pT) );
      jn->SetInputs(7, norm_cat_eta(cat_eta) );
    }

    jn->Evaluate();

   float bvalue=jn->GetOutput(0);
   float cvalue=jn->GetOutput(1);

    if (bottom==1)
    {
      if (i%dilutionFactor==0)
      {
        c_sig->Fill(bvalue/(bvalue+cvalue),weight);
      }
      else if (i%dilutionFactor==1)
      {
        c_sigtest->Fill(bvalue/(bvalue+cvalue),weight);
      }
    }
    if (light==1)
    {
      if (i%dilutionFactor==0)
      {
        c_bg->Fill(bvalue/(bvalue+cvalue),weight);
      }
      else if (i%dilutionFactor==1)
      {
        c_bgtest->Fill(bvalue/(bvalue+cvalue),weight);
      }
    }
    if (charm==1)
    {
      if (i%dilutionFactor==0)
      {
        c_bg2->Fill(bvalue/(bvalue+cvalue),weight);
      }
      else  if (i%dilutionFactor==1)
      {
        c_bg2test->Fill(bvalue/(bvalue+cvalue),weight);
      }
    }
   }

  //now you need the maximum
 maximum=1;
  for (Int_t a=0;a<c_bg->GetNbinsX();a++)
  {
    if (c_bg->GetBinContent(a)>maximum)
    {
      maximum=1.2*c_bg->GetBinContent(a);
    }
  }

   c_bg2->SetLineColor(kYellow);
   c_bg2->SetFillStyle(3008);   c_bg2->SetFillColor(kYellow);
   c_bg->SetLineColor(kBlue);
   c_bg->SetFillStyle(3008);   c_bg->SetFillColor(kBlue);
   c_sig->SetLineColor(kRed);
   c_sig->SetFillStyle(3003); c_sig->SetFillColor(kRed);
   c_bg2->SetStats(0);
   c_bg->SetStats(0);
   c_sig->SetStats(0);
 
   c_bg2test->SetLineColor(kYellow);
   c_bg2test->SetFillStyle(3008);   c_bg2test->SetFillColor(kYellow);
   c_bgtest->SetLineColor(kBlue);
   c_bgtest->SetFillStyle(3008);   c_bgtest->SetFillColor(kBlue);
   c_sigtest->SetLineColor(kRed);
   c_sigtest->SetFillStyle(3003); c_sigtest->SetFillColor(kRed);
   c_bg2test->SetStats(0);
   c_bgtest->SetStats(0);
   c_sigtest->SetStats(0);

   mlpa_canvas->cd(3);
   gPad->SetLogy();


   c_bg->GetYaxis()->SetRangeUser(1,maximum);
   c_bgtest->GetYaxis()->SetRangeUser(1,maximum);
   
   c_bg->Draw();
   c_bg2->Draw("same");
   c_sig->Draw("same");

   TLegend *legend2 = new TLegend(.75, .80, .95, .95);
   legend2->AddEntry(c_bg2, "Background2 (charm)");
   legend2->AddEntry(c_bg, "Background (light)");
   legend2->AddEntry(c_sig, "Signal (bottom)");
   legend2->Draw();

   mlpa_canvas->cd(4);
   gPad->SetLogy();
   
   c_bgtest->Draw();
   c_bg2test->Draw("same");
   c_sigtest->Draw("same");

   TLegend *legend2test = new TLegend(.75, .80, .95, .95);
   legend2test->AddEntry(c_bg2test, "Background2 (charm)");
   legend2test->AddEntry(c_bgtest, "Background (light)");
   legend2test->AddEntry(c_sigtest, "Signal (bottom)");
   legend2test->Draw();

   mlpa_canvas->cd(7);
   gPad->SetLogy();
   c_bg->DrawNormalized();
   c_bg2->DrawNormalized("same");
   c_sig->DrawNormalized("same");
   legend2->Draw();
 
   mlpa_canvas->cd(8);
   gPad->SetLogy();
   c_bgtest->DrawNormalized();
   c_bg2test->DrawNormalized("same");
   c_sigtest->DrawNormalized("same");
   legend2test->Draw();


   mlpa_canvas->cd(0);

   mlpa_canvas->SaveAs("weights/result.eps");
}

