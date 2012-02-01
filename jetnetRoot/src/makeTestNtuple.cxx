#include "makeTestNtuple.hh"
#include "readReducedDataset.hh"
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "testJetNet.hh"
#include "getPtEtaCategoryLikelihood.hh"
#include "TRandom.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <TH1D.h>
#include "doNormalization.hh"
#include "TTrainedNetwork.h"
#include "TJetNet.h"
#include <TVectorD.h>
#include <TMatrixD.h>
#include <string> 
#include "nnExceptions.hh"

using namespace std;


void makeTestNtuple(std::string input_weights_name,
		    std::string input_dataset_name, 
		    std::string output_file_name, 
		    std::string output_tree_name)

{

  TFile input_weights_file(input_weights_name.c_str());
  TTrainedNetwork* trainedNetwork = dynamic_cast<TTrainedNetwork*>
    (input_weights_file.Get("TTrainedNetwork"));

  
  if (!trainedNetwork) 
  {
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
  
  for (Int_t o=0;o<nHidden;++o)
  {
    cout << " Hidden lay: " << o << " size: " << nHiddenLayerSize[o];
  }
  Int_t nOutput=trainedNetwork->getnOutput();
  
  cout << " Output size: " << nOutput << endl;
  
  
  //now calculate the value using:
  TVectorD** resultVector=new TVectorD*[nHidden+1];
  
  for (Int_t o=0;o<nHidden+1;++o)
  {
    int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;
    int sizePreviousLayer=(o==0)?nInput:nHiddenLayerSize[o-1];
    resultVector[o]=new TVectorD(sizeActualLayer);
  }

  std::vector<TVectorD*> thresholdVectors=trainedNetwork->getThresholdVectors();
  std::vector<TMatrixD*> weightMatrices=trainedNetwork->weightMatrices();
 
  //END HERE

  //CROSS CHECK HERE

  TJetNet* jn = new TJetNet( 3000, 7000, nLayers, nneurons );

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

  



  Int_t nVTX;
  Int_t nTracksAtVtx;
  Int_t nSingleTracks;
  Double_t energyFraction;
  Double_t mass;
  Double_t significance3d;

  
  Int_t cat_flavour;

  Double_t discriminatorIP2D;
  Double_t discriminatorIP3D;
  Double_t discriminatorSV1;
  Double_t discriminatorCOMB;
 
  Double_t weight;
 
  Double_t deltaR;
  Double_t JetPt;
  Double_t JetEta;
  Int_t cat_pT;
  Int_t cat_eta;

  Double_t NNb;
  Double_t NNc;
  Double_t NNu;
  
  

  cout << " file name to open is: " << input_dataset_name << endl;

  TFile file(input_dataset_name.c_str());

  TString inputTreeName=("SVTree");

  TTree* inputTree = dynamic_cast<TTree*>(file.Get(inputTreeName));
  if (! inputTree){ 
    throw LoadReducedDSException(); 
  }

  readReducedDataset* readTreeJF=new readReducedDataset(inputTree);

  TFile* outputFile=new TFile(output_file_name.c_str(),"recreate");


  TTree* myTree=new TTree(output_tree_name.c_str(),output_tree_name.c_str());

  if (!true) // longterm: make out variables read in from a list 
  {
    myTree->Branch("nVTX",&nVTX,"nVTX/I");
    myTree->Branch("nTracksAtVtx",&nTracksAtVtx,"nTracksAtVtx/I");
    myTree->Branch("nSingleTracks",&nSingleTracks,"nSingleTracks/I");
    myTree->Branch("energyFraction",&energyFraction,"energyFraction/D");
    myTree->Branch("mass",&mass,"mass/D");
    myTree->Branch("significance3d",&significance3d,"significance3d/D");
  }
  
  myTree->Branch("cat_pT",&cat_pT,"cat_pT/I");
  myTree->Branch("cat_eta",&cat_eta,"cat_eta/I");
  myTree->Branch("weight",&weight,"weight/D");

  myTree->Branch("cat_flavour",&cat_flavour,"cat_flavour/I");
  myTree->Branch("deltaR",&deltaR,"deltaR/D");    
  myTree->Branch("JetPt",&JetPt,"JetPt/D");
  myTree->Branch("JetEta",&JetEta,"JetEta/D");

  myTree->Branch("discriminatorIP3D",&discriminatorIP3D,"discriminatorIP3D/D");
  myTree->Branch("discriminatorIP2D",&discriminatorIP2D,"discriminatorIP2D/D");
  myTree->Branch("discriminatorSV1",&discriminatorSV1,"discriminatorSV1/D");
  myTree->Branch("discriminatorCOMB",&discriminatorCOMB,"discriminatorCOMB/D");

  myTree->Branch("NNb",&NNb,"NNb/D");
  myTree->Branch("NNc",&NNc,"NNc/D");
  myTree->Branch("NNu",&NNu,"NNu/D");
  
  Int_t num_entries=readTreeJF->fChain->GetEntries();


  cout << "Total entries are: " << num_entries << endl;

  for (Int_t i=0;i<num_entries;++i)
 {


    if (i % 500000 == 0 ) {
      std::cout << " processing event number " << i << std::endl;
    }
    
    readTreeJF->GetEntry(i);


    JetPt=readTreeJF->JetPt;
    JetEta=readTreeJF->JetEta;
    cat_pT=getPtCategory(JetPt).first;
    cat_eta=getEtaCategory(JetEta).first;
    cat_flavour=readTreeJF->cat_flavour;
    weight=readTreeJF->weight;
    
    discriminatorIP3D=readTreeJF->discriminatorIP3D;
    discriminatorIP2D=readTreeJF->discriminatorIP2D;
    discriminatorSV1=readTreeJF->discriminatorSV1;
    discriminatorCOMB=readTreeJF->discriminatorCOMB;
    deltaR=readTreeJF->deltaR;

    float input[9]=
        {   norm_nVTX(readTreeJF->nVTX),
            norm_nTracksAtVtx(readTreeJF->nTracksAtVtx),
            norm_nSingleTracks(readTreeJF->nSingleTracks),
            norm_energyFraction(readTreeJF->energyFraction),
            norm_mass(readTreeJF->mass),
            norm_significance3d(readTreeJF->significance3d),
            norm_IP3D(readTreeJF->discriminatorIP3D),
            norm_cat_pT(cat_pT),
            norm_cat_eta(cat_eta) };


    if (i<10)
    {
      cout << " input variables: ";
      for (int o=0;o<9;++o)
      {
        cout << "var " << o << ": " << input[o] << endl;
      }
    }

    //CALCULATION STARTS HERE

    for (Int_t o=0;o<nHidden+1;++o)
    {
      int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;
      int sizePreviousLayer=(o==0)?nInput:nHiddenLayerSize[o-1];
  
      for (Int_t s=0;s<sizeActualLayer;++s)
      {
	// TODO: change this ->operator() syntax
        Double_t nodeValue=0.;
        if (o==0)
        {
          for (Int_t p=0;p<nInput;++p)
          {
            nodeValue+=weightMatrices[o]->operator() (p,s)*input[p];
          }
        }
        else
        {
          for (Int_t p=0;p<nHiddenLayerSize[o-1];++p)
          {
            nodeValue+=weightMatrices[o]->operator() (p,s)*resultVector[o-1]->operator()(p);
          }
        }
        nodeValue+=thresholdVectors[o]->operator() (s);
        resultVector[o]->operator()(s) = sigmoid(nodeValue);
      }
  }      
  
  TVectorD result(nOutput);
  for (Int_t jjj = 0; jjj < nOutput; jjj++)
  {
    result.operator()(jjj)=
      resultVector[nHidden]->operator()(jjj);
  }

  //CALCULATION ENDS HERE

  NNb=result.operator()(0);
  NNc=result.operator()(1);
  NNu=result.operator()(2);
    
  if (i<10)
  {
    cout << " NNb: " << NNb << " NNc: " << NNc << " NNu: " << NNu << endl;
  }
  
  //cross check

   for (Int_t i=0;i<trainedNetwork->getnInput();++i)
  {
    jn->SetInputs(i,input[i]);
  }

  jn->Evaluate();

//  cout << "Result 0:" << jn->GetOutput(0);
//  cout << " Result 1:" << jn->GetOutput(1);
//  cout << " Result 2:" << jn->GetOutput(2) << endl;

  if (fabs(NNb-jn->GetOutput(0))>1e-4)
  {
    cout << " Different: NNb: " << NNb << " output0: " << jn->GetOutput(0) << endl;
  }
  
  if (fabs(NNc-jn->GetOutput(1))>1e-4)
  {
    cout << " Different: NNc: " << NNc << " output0: " << jn->GetOutput(1) << endl;
  }

  if (fabs(NNu-jn->GetOutput(2))>1e-4)
  {
    cout << " Different: NNu: " << NNu << " output0: " << jn->GetOutput(2) << endl;
  }

  //end cross check

/*
  nVTX=readTreeJF->nVTX;
  nSingleTracks=readTreeJF->nSingleTracks;
  nTracksAtVtx=readTreeJF->nTracksAtVtx;
  energyFraction=readTreeJF->energyFraction;
  mass=readTreeJF->mass;
  significance3d=readTreeJF->significance3d;
*/

    
    myTree->Fill();
  }


  outputFile->WriteTObject(myTree); 

  // outputFile->Write();
  outputFile->Close();

  
}
  
