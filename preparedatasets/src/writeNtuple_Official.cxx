#include "readBaseBTagAnaTree.hh"
#include "readJFBTagAna.hh"
#include <TFile.h>
#include <TTree.h>
#include <iostream>
#include "writeNtuple_Official.hh"
#include "getPtEtaCategoryLikelihood.hh"
#include "TRandom.h"
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

struct number_pair
{
  int first;
  double second;
  number_pair(int p1, double p2)
    : first (p1), second (p2) {}
  bool operator< (const number_pair& other) const
  { return second > other.second; }
};

struct allinfos
{
  Int_t nVTX;
  Int_t nTracksAtVtx;
  Int_t nSingleTracks;
  Double_t energyFraction;
  Double_t mass;
  Double_t significance3d;

  Int_t cat_flavour;
  Int_t bottom;
  Int_t charm;
  Int_t light;

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
};




void writeNtuple_Official(SVector input_files, 
			  SVector observer_discriminators, 
                          std::string jetCollectionName,
                          std::string suffix,
                          bool forNN,
                          bool randomize) 
{
  

  std::string jetCollection = jetCollectionName + suffix;

  cout << " opening input file: " << inputFileName << endl 
       << " processing to obtain: " << jetCollection 
       << " root file "  << endl;
  
  std::string baseBTag("BTag_");

  // --- observer variables
  std::vector<readBaseBTagAnaTree*> observer_arrays; 
  std::vector<TChain*> observer_chains; 
  
  for (SVector::const_iterator name_itr = observer_discriminators.begin(); 
       name_itr != observer_discriminators.end(); 
       name_itr++){ 
    cout << "instantiating " << *name_itr << endl;
    std::string chain_name = baseBTag + jetCollection + 
      "_" + *name_itr + "/PerfTreeAll"; 
    TChain* the_chain = new TChain(chain_name.c_str()); 
    observer_chains.push_back(the_chain); 
    

    for (SVector::const_iterator in_file_itr = input_files.begin(); 
	 in_file_itr != input_files.end(); 
	 in_file_itr++){ 
      (*chain_itr)->Add(*in_file_itr); 
    }

    if (the_chain->GetEntries() == 0)
      throw LoadOfficialDSException();
    
    observer_arrays.push_back(new readBaseBTagAnaTree(the_chain) ); 

  }

  // --- jetfitter variables
  std::string suffixJF("_JetFitterTagNN/PerfTreeAll");
  cout << "instantiating JetFitterTagNN " << endl;
  TChain* treeJF=new TChain((baseBTag+jetCollection+suffixJF).c_str());

  for (SVector::const_iterator in_file_itr = input_files.begin(); 
       in_file_itr != input_files.end(); 
       in_file_itr++){ 
    treeJF->Add(*in_file_itr); 
  }

  if (treeJF->GetEntries()==0) throw std::string("Problem JF");
  readJFBTagAna* readTreeJF= new readJFBTagAna(treeJF);

  // TString suffixIP2D("_IP2D/PerfTreeAll");
  // TString suffixIP3D("_IP3D/PerfTreeAll");
  // TString suffixSV1("_SV1/PerfTreeAll");
  // TString suffixCOMB("_COMB/PerfTreeAll");



  //for the NN you need to get the number of b,c or light jets

  printf("counting entries, will take a while\n"); 
  Int_t num_entries=readTreeJF->fChain->GetEntries();

  int numberb=0;
  int numberc=0;
  int numberl=0;

  if (forNN) 
  {

    for (Long64_t i=0;i<num_entries;i++) {

      readTreeJF->GetEntry(i);
      
      if (readTreeJF->mass > -100)
      {
        if (abs(readTreeJF->Flavour)==5){
          numberb+=1;
        }
        if (abs(readTreeJF->Flavour)==4){
          numberc+=1;
        }
        if (abs(readTreeJF->Flavour==1)){
          numberl+=1;
        }
      }
    }
  }
  
  //now you have to calculate the weights...
  //(store them in a matrix for b,c or light jets...

  cout << " number of b found : " << numberb << " c: " << numberc << " l: " << numberl << endl;

  double correctionfactor=1;

  
  int numPtBins=getNPtBins();
  int numEtaBins=getNEtaBins();

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
  
  
  if (forNN) 
  {
    
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
      
      readTreeJF->GetEntry(i);
      
      if (readTreeJF->mass<-100) continue;
      
      if (fabs(readTreeJF->JetEta)>2.5||readTreeJF->JetPt<=15.)  continue;

      
      pair<int,double> ptInfo=getPtCategory(readTreeJF->JetPt);
      pair<int,double> etaInfo=getEtaCategory(readTreeJF->JetEta);

      int actualpT=ptInfo.first;
      int actualeta=etaInfo.first;
      
      int flavour=abs(readTreeJF->Flavour);

//      cout << " actualpT " << actualpT << " actualeta " << actualeta << endl;
      
      switch (flavour)
      {
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
//      cout << " for i=ptxeta = " << i << " weightsb[i] " << weightsb[i]
//           << " weightsl[i] " << weightsl[i] << " weightsc[i] " << weightsc[i] << endl;
      weightsb[i]= (Double_t)numberb / (Double_t)countb[i] ;
      weightsl[i]= (Double_t)numberl / (Double_t)countl[i] ;
      weightsc[i]= (Double_t)numberc / (Double_t)countc[i];

      if (weightsb[i]>maxweightb) maxweightb=weightsb[i];
      if (weightsl[i]>maxweightl) maxweightl=weightsl[i];
      if (weightsc[i]>maxweightc) maxweightc=weightsc[i];

    }
  
  }
  

  cout << " maxweightb: " << maxweightb << " maxweightc: " << maxweightc << 
      " maxweightl: " << maxweightl << endl;

  TFile* file=0;
  
  TString filename("../reduceddatasets/reduceddataset_");
  filename+=jetCollectionName;
  if (forNN)
  {
    filename+="_forNN";
  }
  filename+=".root";

  file=new TFile(filename,"recreate");
  
  TTree* myTree=new TTree("SVTree","SVTree");

  // --- things to write out
  Int_t nVTX;
  Int_t nTracksAtVtx;
  Int_t nSingleTracks;
  Double_t energyFraction;
  Double_t mass;
  Double_t significance3d;

  Int_t cat_flavour;
  Int_t bottom;
  Int_t charm;
  Int_t light;

  // observer write branches 
  std::vector<double*> observer_write_buffers; 
  for (SVector::const_iterator name_itr = observer_discriminators.begin(); 
       name_itr != observer_discriminators.end(); 
       name_itr++){ 
    double* the_buffer = new double; 
    observer_write_buffers.push_back(the_buffer); 
    std::string name = "discriminator" + *name_itr; 
    myTree->Branch(name.c_str(), the_buffer); 
  }
    
  // Double_t discriminatorIP2D;
  // Double_t discriminatorIP3D;
  // Double_t discriminatorSV1;
  // Double_t discriminatorCOMB;
 
  Double_t weight;
 
  Double_t deltaR;
  Double_t JetPt;
  Double_t JetEta;
  Int_t cat_pT;
  Int_t cat_eta;


  
  myTree->Branch("nVTX",&nVTX,"nVTX/I");
  myTree->Branch("nTracksAtVtx",&nTracksAtVtx,"nTracksAtVtx/I");
  myTree->Branch("nSingleTracks",&nSingleTracks,"nSingleTracks/I");
  myTree->Branch("energyFraction",&energyFraction,"energyFraction/D");
  myTree->Branch("mass",&mass,"mass/D");
  myTree->Branch("significance3d",&significance3d,"significance3d/D");

  
  if (forNN){
    myTree->Branch("cat_pT",&cat_pT,"cat_pT/I");
    myTree->Branch("cat_eta",&cat_eta,"cat_eta/I");
    myTree->Branch("weight",&weight,"weight/D");

    myTree->Branch("bottom",&bottom,"bottom/I");
    myTree->Branch("charm",&charm,"charm/I");
    myTree->Branch("light",&light,"light/I");
  }
  


  myTree->Branch("deltaR",&deltaR,"deltaR/D");    
  myTree->Branch("JetPt",&JetPt,"JetPt/D");
  myTree->Branch("JetEta",&JetEta,"JetEta/D");
  myTree->Branch("cat_flavour",&cat_flavour,"cat_flavour/I");  


//  if (num_entries!=readTreeCOMB->fChain->GetEntries()) {
//    throw std::string("event number doesn't match");
//  }

  //now randomize entries

/*
  std::vector<int> inputvalues;
  
  for (Int_t i=0;i<num_entries;i++) 
  {
    inputvalues.push_back(i);
  }
  
  std::vector<int> outputvalues;
  int counter=0;
  do
  {

    int size=inputvalues.size();
    if (size==0)
    {
      break;
    }
    double randomNumber=random.Uniform(0,size);
    int nToTransfer=(int)(std::floor(randomNumber<size?randomNumber:0)+0.5);
    vector<int>::iterator begin=inputvalues.begin();
    begin+=nToTransfer;
    outputvalues.push_back(inputvalues[nToTransfer]);
    if (counter%100000==0)
    {
    cout << " Processing randomizing event: " << counter << 
        " inputvalue " << inputvalues[nToTransfer] <<  endl;
    }
    
    inputvalues.erase(begin);
    counter+=1;


  }
  while (true);
*/

  std::vector<number_pair> outputvalues;
  
  for (Int_t i=0;i<num_entries;i++) 
  {
    outputvalues.push_back(number_pair(i,random.Uniform()));
  }
  
  if (randomize)
  {
    
    cout << " Doing sorting... " << endl;
    std::sort (outputvalues.begin(), outputvalues.end());
    cout << " End sorting ... " << endl;
  }
  

  cout << "Total entries are: " << num_entries << endl;
  Int_t i=0;

  
  vector<number_pair>::const_iterator begin=outputvalues.begin();
  vector<number_pair>::const_iterator end=outputvalues.end();
  
  Int_t counter=0;
  for (vector<number_pair>::const_iterator iter=begin;iter!=end;++iter)
  {
    i=(*iter).first;

    //take only every fifth data point
    if (!forNN)
    {
      if (counter%5 != 0)
      {
        counter+=1;
        continue;
      }
    }
    

//    if (counter>1000000)
//      break;
//    }
    

    if (counter % 500000 == 0 ) {
      std::cout << " processing event number " << counter << " data event: " << i << " which was event n. " << std::endl;
    }
    
    counter+=1;
     
    
    readTreeJF->GetEntry(i);


    if (fabs(readTreeJF->JetEta) < 2.5 &&
	readTreeJF->JetPt > 15.0 &&
	readTreeJF->mass > -100) {


      JetPt=readTreeJF->JetPt;
      JetEta=readTreeJF->JetEta;
      cat_pT=getPtCategory(JetPt).first;
      cat_eta=getEtaCategory(JetEta).first;
      
      cat_flavour=abs(readTreeJF->Flavour);
      if (forNN)
      {
        bottom=0;
        charm=0;
        light=0;

        bool throwevent(false);

        switch (cat_flavour)
        {
        case 5:
          bottom=1;
          weight=weightsb[cat_pT+numPtBins*cat_eta];

          if (forNN)
          {
            
            if (weight<maxweightb/toleranceb)
            {
              if (random.Uniform()>weight*toleranceb/maxweightb)
              {
                throwevent=true;
              }
              weight=1.;//maxweightb/toleranceb;
            }
            else
            {
              weight/=(maxweightb/toleranceb);
            }
          } 
            
          
          break;
        case 4:
          charm=1;
          weight=weightsc[cat_pT+numPtBins*cat_eta];

          if (forNN)
          {
            if (weight<maxweightc/tolerancec)
            {
              if (random.Uniform()>weight*tolerancec/maxweightc)
              {
                throwevent=true;
              }
              weight=1.;//maxweightc/tolerancec;
            }
            else
            {
              weight/=(maxweightc/tolerancec);
            }
          }
          


          break;
        case 1:
          light=1;
          weight=weightsl[cat_pT+numPtBins*cat_eta];

          if (forNN)
          {
            
            if (weight<maxweightl/tolerancel)
            {
              if (random.Uniform()>weight*tolerancel/maxweightl)
              {
                throwevent=true;
              }
              weight=1.;//maxweightl/tolerancel;
            }
            else 
            {
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
      for (short i = 0; i < observer_chains.size(); i++){ 
	TChain* the_chain = observer_chains.at(i); 
	readBaseBTagAnaTree* read_buffer = observer_arrays.at(i); 
	double* write_buffer = observer_write_buffers.at(i); 

	the_chain->GetEntry(i); 
	// copy to the write buffer
	*write_buffer = read_buffer->Discriminator; 
	
      }

      // discriminatorIP3D=readTreeIP3D->Discriminator;
//          readTreeIP3D->Discriminator>-10?readTreeIP3D->Discriminator:-10;
      
//      if (!forNN)
//      {
        // discriminatorIP2D=readTreeIP2D->Discriminator;
        // discriminatorSV1=readTreeSV1->Discriminator;
        // discriminatorCOMB=readTreeCOMB->Discriminator;
        // deltaR=readTreeCOMB->DeltaRtoBorCorTau;
//      }
      

      nVTX=readTreeJF->nVTX;
      nSingleTracks=readTreeJF->nSingleTracks;
      nTracksAtVtx=readTreeJF->nTracksAtVtx;
      energyFraction=readTreeJF->energyFraction;
      mass=readTreeJF->mass;
      significance3d=readTreeJF->significance3d;
      myTree->Fill();
      
    }
    
  }

  file->WriteTObject(myTree); 
  file->Close();
  


  /*
  delete readTreeIP2D;
  delete readTreeIP3D;
  delete readTreeSV1;
  delete readTreeCOMB;
  delete readTreeJF;
  */

//  inputFile.Close();

  
}
  
