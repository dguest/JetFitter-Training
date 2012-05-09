#include "JetNet.hh"
#include "jetnet.h"
#include "TRandom3.h"
#include "TTimeStamp.h"
#include <unistd.h>
#include <stdio.h>
#include <cmath>
#include "TFile.h"

#include "TH1F.h"

// ClassImp( JetNet )
 
//Constructors
//______________________________________________________________________________
JetNet::JetNet()
{
  // Default Constructor
  mTestSetCnt = 0;
  mTrainSetCnt = 0;
  mLayerCount = 0;

  mpLayers = 0;

  mIsInitialized = kFALSE;
  mInitLocked = kFALSE;

  mpInputTrainSet = 0;
  mpInputTestSet = 0;
  mpOutputTrainSet = 0;
  mpOutputTestSet = 0;

  mEpochs = 0;
  mCurrentEpoch = 0;
}
//______________________________________________________________________________
JetNet::JetNet( Int_t aTestCount, Int_t aTrainCount,
	 const Int_t aLayersCnt, const Int_t* aLayers )
{
  // Creates neural network with aLayersCnt number of layers,
  // aTestCount number of patterns for the test set,
  // aTrainCount patterns for the train set.
  // aLayers contains the information for number of the units in the different layers

  int i;
  mDebug = kFALSE;
#ifdef _DEBUG
  mDebug = kTRUE;
#endif
  
  if( mDebug ){ std::cout << "=====> Entering JetNet::JetNet(...)" << std::endl; }

  mTestSetCnt  = aTestCount;
  mTrainSetCnt = aTrainCount;
  mLayerCount  = aLayersCnt; // Get the number of layers
  
  if( mLayerCount > 0 )
  {  
   //Perform deep copy of the array holding Layers count
    mpLayers = new Int_t[ mLayerCount ];
    for( i = 0; i < mLayerCount; ++i )
    {
      mpLayers[ i ] = aLayers[ i ];
    }
  }

  mInputDim = mpLayers[ 0 ];
  mOutputDim = mpLayers[ mLayerCount - 1 ];
  mHiddenLayerDim = mLayerCount-2;
  

  mIsInitialized = kFALSE;
  mInitLocked = kFALSE;

  mpInputTrainSet  = new TNeuralDataSet( mTrainSetCnt, GetInputDim() );
  mpInputTestSet = new TNeuralDataSet( mTestSetCnt, GetInputDim() );
  mpOutputTrainSet = new TNeuralDataSet( mTrainSetCnt, GetOutputDim() );
  mpOutputTestSet = new TNeuralDataSet( mTestSetCnt, GetOutputDim() );
  
  menActFunction=afSigmoid;

  SetEpochs( -1 );

  if( mDebug ){ std::cout << "=====> Leaving JetNet::JetNet(...)" << std::endl; }
}
//______________________________________________________________________________
JetNet::~JetNet( void )
{
  // Default destructor
  if( mDebug ){ std::cout << "=====> Entering JetNet::~JetNet(...)" << std::endl; }
  delete [] mpLayers;
  delete mpInputTestSet;
  delete mpInputTrainSet;
  delete mpOutputTestSet;
  delete mpOutputTrainSet;
  if( mDebug ){ std::cout << "=====> Leaving JetNet::~JetNet(...)" << std::endl; }
}
//______________________________________________________________________________
//______________________________________________________________________________

void JetNet::setInputNodes(std::vector<JetNet::InputNode> v) { 
  m_input_nodes = v; 
}
std::vector<JetNet::InputNode> JetNet::getInputNodes() const { 
  return m_input_nodes; 
}

  
void JetNet::SetWeight( Double_t weight,Int_t aLayerInd, Int_t aNodeInd, Int_t aConnectedNodeInd )
{
  JNINT1.W[ JNINDX( aLayerInd, aNodeInd, aConnectedNodeInd )-1 ]=weight;
}
//______________________________________________________________________________
void JetNet::SetThreshold( Double_t threshold, Int_t aLayerInd, Int_t aNodeInd)
{
  JNINT1.T[ JNINDX( aLayerInd, aNodeInd, 0 )-1 ]=threshold;
}
//______________________________________________________________________________
void JetNet::Print( void )
{
  // Prints on the screen, information for the neural network
  Int_t i;

  std::cout << "JetNet" << std::endl;
  std::cout << "Number of layers: " << mLayerCount << std::endl;

  for( i = 0; i < mLayerCount; i++ )
  {
      std::cout << "\t\tNumber of units in layer " << i << " : " <<  mpLayers[ i ] << std::endl;
  }

  std::cout << "Epochs: " << GetEpochs() << std::endl;
  std::cout << "Updates Per Epoch: " << GetUpdatesPerEpoch() << std::endl;
  std::cout << "Updating Procedure: " << GetUpdatingProcedure() << std::endl;
  std::cout << "Error Measure: " << GetErrorMeasure() << std::endl;
  std::cout << "Patterns Per Update: " << GetPatternsPerUpdate() << std::endl;
  std::cout << "Learning Rate: " << GetLearningRate() << std::endl;
  std::cout << "Momentum: " << GetMomentum() << std::endl;
  std::cout << "Initial Weights Width: " << GetInitialWeightsWidth() << std::endl;
  std::cout << "Learning Rate Decrease: " << GetLearningRateDecrease() << std::endl;
  std::cout << "Activation Function: " << GetActivationFunction() << std::endl;
}
//______________________________________________________________________________
Double_t JetNet::Test( void )
{
  // Initiate test cycle of the neural network
  Int_t NRight = 0;
  Double_t fMeanError = 0.0;
  Double_t *TMP;
  Int_t  NPatterns = GetTestSetCnt();
 

  for( Int_t iPattern = 0; iPattern < NPatterns; iPattern++ )
  {

      for( Int_t i = 0; i < GetInputDim(); i++ )
      {
	JNDAT1.OIN[ i ] = float ( GetInputTestSet( iPattern, i ) );
      }

      NWJNWGT.OWGT = GetEventWeightTestSet( iPattern );

      JNTEST();

      for( Int_t j = 0; j < GetOutputDim(); j++ )
      {
	fMeanError+= NWJNWGT.OWGT * 
	  std::pow(JNDAT1.OUT[ j ]-float( GetOutputTestSet( iPattern, j )),2)/(float)GetOutputDim();
      }



      if( mDebug ) std::cout << "Testing [ " << iPattern << " ] - "  << JNDAT1.OIN[ 0 ] 
			     << " => " << JNDAT1.OUT[ 0 ] << std::endl;

    }

  fMeanError/=2.*NPatterns;

  if (mDebug)
  std::cout << " Test error: " << fMeanError << endl;

  return fMeanError;
}
//
Double_t JetNet::TestBTAG( void )
{

  bool test=false;

  // Initiate test cycle of the neural network
  Int_t NRight = 0;
  Double_t fMeanError = 0.0;
  Double_t *TMP;
  Int_t  NPatterns = GetTestSetCnt();
  if (test)
  {
    NPatterns = GetTrainSetCnt();
  }
 
  vector<double> eff;
  eff.push_back(0.3);
  eff.push_back(0.5);
//  eff.push_back(0.6);
  eff.push_back(0.7);

  //test also the b-tagging performance directly during testing !!!
  vector<TH1F*> histoEfficienciesC;
  vector<TH1F*> histoEfficienciesL;
  TString histoEffStringC("histoEffC");
  TString histoEffStringL("histoEffL");
  for (int i=0;i<GetOutputDim();i++)
  {
    TString string1=histoEffStringC;
    string1+=i;
    TH1F* histo=new TH1F(string1,
                         string1,
                         20000,
                         -2,3);
    TString string2=histoEffStringL;
    string2+=i;
    TH1F* histo2=new TH1F(string2,
                          string2,
                          20000,
                          -2,3);
    histoEfficienciesC.push_back(histo);
    histoEfficienciesL.push_back(histo2);
  }

  for( Int_t iPattern = 0; iPattern < NPatterns; iPattern++ )
  {

      for( Int_t i = 0; i < GetInputDim(); i++ )
      {
        if (!test)
        {
          JNDAT1.OIN[ i ] = float ( GetInputTestSet( iPattern, i ) );
          NWJNWGT.OWGT = GetEventWeightTestSet( iPattern );
        }
        else
        {
          JNDAT1.OIN[ i ] = float (GetInputTrainSet( iPattern, i ) );
          NWJNWGT.OWGT = GetEventWeightTrainSet( iPattern );
        }

      }

      JNTEST();

      int active=0;
      for( Int_t j = 0; j < GetOutputDim(); j++ )
      {
        if (!test)
        {
          fMeanError+= NWJNWGT.OWGT * 
              std::pow(JNDAT1.OUT[ j ]-float( GetOutputTestSet( iPattern, j )),2)/(float)GetOutputDim();
        }
        else
        {
           fMeanError+= NWJNWGT.OWGT * 
               std::pow(JNDAT1.OUT[ j ]-float( GetOutputTrainSet( iPattern, j )),2)/(float)GetOutputDim();
        }
        

//        std::cout << " j " <<  j << " is " << GetOutputTestSet( iPattern, j) << std::endl;

        if (!test)
        {
          if (fabs(float( GetOutputTestSet( iPattern, j)) - 1) < 1e-4)
          {
            active = j;
          }
        }
        else
        {
          if (fabs(float( GetOutputTrainSet( iPattern, j)) - 1) < 1e-4)
          {
            active = j;
          }
        }
      }

//      if (mDebug) std::cout << " active is: " << active << std::endl;

//      if (mDebug) std::cout << " filling histograms " << std::endl;

      if (JNDAT1.OUT[ 0 ] + JNDAT1.OUT[ 1 ] >= 0)
      {
        histoEfficienciesC[active]->Fill( JNDAT1.OUT[ 0 ] / 
                                          ( JNDAT1.OUT[ 0 ] + JNDAT1.OUT[ 1 ]),
                                          NWJNWGT.OWGT);
                                                                                
//        if( mDebug ) std::cout << "Filled: " << JNDAT1.OUT[ 0 ] / 
//                         ( JNDAT1.OUT[ 0 ] + JNDAT1.OUT[ 2 ]) << std::endl;
      
      }
      else
      {
        std::cout << " Filled 0 " << std::endl;
        histoEfficienciesC[active]->Fill( 0 );
      }
      
        
      if (JNDAT1.OUT[ 0 ] + JNDAT1.OUT[ 2 ] >= 0)
      {
        histoEfficienciesL[active]->Fill( JNDAT1.OUT[ 0 ] / 
                                          ( JNDAT1.OUT[ 0 ] + JNDAT1.OUT[ 2 ]),
                                          NWJNWGT.OWGT);
//        if( mDebug ) std::cout << "Filled: " << JNDAT1.OUT[ 0 ] / 
//                         ( JNDAT1.OUT[ 0 ] + JNDAT1.OUT[ 1 ]) << std::endl;
      
     }
      else
      {
        std::cout << " Filled 0 " << std::endl;
        histoEfficienciesL[active]->Fill( 0 );
      }

      if( mDebug ) std::cout << "Testing [ " << iPattern << " ] - "  << JNDAT1.OIN[ 0 ] 
                             << " => " << JNDAT1.OUT[ 0 ] << std::endl;
      
  }// finish patterns
  
  if (mDebug) std::cout << " Finished patterns... " << std::endl;

  TFile* newFile=new TFile("test.root","recreate");
  histoEfficienciesL[0]->Write();
  histoEfficienciesL[1]->Write();
  histoEfficienciesL[2]->Write();
  histoEfficienciesC[0]->Write();
  histoEfficienciesC[1]->Write();
  histoEfficienciesC[2]->Write();
  // newFile->Write();
  newFile->Close();
  delete newFile; 
  newFile = 0; 

  //for C-jet rejection
  
  for (int u=0;u<2;u++)
  {
    vector<TH1F*>* myVectorHistos;
    if (u==0) 
    {
      std::cout << "c-rej --> ";
      myVectorHistos=&histoEfficienciesC;
    }
    if (u==1)
    {
      std::cout << "l-rej --> ";
      myVectorHistos=&histoEfficienciesL;
    }
    
      
    if (mDebug) std::cout << " 1 " << std::endl;
    
    Double_t allb=(*myVectorHistos)[0]->GetSumOfWeights();
    Double_t allc=(*myVectorHistos)[1]->GetSumOfWeights();
    Double_t allu=(*myVectorHistos)[2]->GetSumOfWeights();
    
    if (mDebug) std::cout << " allb " << allb << std::endl;

    Double_t allbsofar=0;
    
    vector<int> binN_Eff;
    vector<bool> ok_eff;
    
    for (int r=0;r<eff.size();r++)
    {
      ok_eff.push_back(false);
      binN_Eff.push_back(0);
    }

    for (int s=0;s<(*myVectorHistos)[0]->GetNbinsX()+1;s++) {
      allbsofar+=(*myVectorHistos)[0]->GetBinContent((*myVectorHistos)[0]->GetNbinsX()+1-s);
      bool nothingMore(true);

 
      for (int r=0;r<eff.size();r++)
      {
        if (mDebug) std::cout << " actual eff: " << allbsofar / allb << std::endl;

        if ((!ok_eff[r]) && allbsofar / allb > eff[r])
        {
          binN_Eff[r]=s;
          ok_eff[r]=true;
          if (mDebug) std::cout << " bin: " << s << " eff: " << allbsofar / allb << std::endl;
//          std::cout << " Cut value: " << (*myVectorHistos)[0]->GetBinCenter(s) << std::endl;
        }
        else if (allbsofar / allb <= eff[r])
        {
          nothingMore=false;
        }
      }
      if (nothingMore) break;
    }
    
    
    for (int r=0;r<eff.size();r++)
    {
      
      std::cout << " " << eff[r];

      std::cout << " check: " << (double)(*myVectorHistos)[0]->Integral((*myVectorHistos)[0]->GetNbinsX()-binN_Eff[r],
                                                                        (*myVectorHistos)[1]->GetNbinsX()+1)
          / (double)allb;
      
      double effc=(*myVectorHistos)[1]->Integral((*myVectorHistos)[0]->GetNbinsX()-binN_Eff[r],
                                                 (*myVectorHistos)[1]->GetNbinsX()+1);
      effc /= allc;
      double effl=(*myVectorHistos)[2]->Integral((*myVectorHistos)[0]->GetNbinsX()-binN_Eff[r],
                                                 (*myVectorHistos)[2]->GetNbinsX()+1);
      effl /= allu;
      
      if (effc!=0)
      {
        std::cout << " c: " << 1/effc;
      }
      if (effl!=0)
      {
        std::cout << " l: " << 1/effl;
      }
      
    }
    std::cout << std::endl;
  }
  
   for( Int_t j = 0; j < GetOutputDim(); j++ )
    {
      delete histoEfficienciesC[j];
      delete histoEfficienciesL[j];
    }
   

  fMeanError/=2.*NPatterns;

  if (mDebug)
  std::cout << " Test error: " << fMeanError << endl;

  return fMeanError;
  }


//______________________________________________________________________________
Double_t JetNet::Train( void )
{
  // Initiate the train phase for the neural network
  Int_t NRight = 0;
  Double_t fMeanError = 0.0;
  Int_t  NPatterns = GetTrainSetCnt();

  //  cout << " NPatterns is: " << NPatterns << endl;

  Int_t inputDim=GetInputDim();
  Int_t outputDim=GetOutputDim();
  Int_t updatesPerEpoch=GetUpdatesPerEpoch();
  Int_t patternsPerUpdate=GetPatternsPerUpdate();
  
  if (updatesPerEpoch*patternsPerUpdate<1./2.*NPatterns) 
  {
    cout << "Using only: " << updatesPerEpoch*patternsPerUpdate << 
        " patterns on available: " << NPatterns << endl;
  } else if (updatesPerEpoch*patternsPerUpdate>NPatterns) 
  {
    cout << " Trying to use " << updatesPerEpoch*patternsPerUpdate << 
        " patterns, but available: " << NPatterns << endl;
    return -100;
  }
  
  for( Int_t iPattern = 0; iPattern < updatesPerEpoch*patternsPerUpdate;
       iPattern++ )
  {
    for( Int_t i = 0; i < inputDim; i++ )
    {
      JNDAT1.OIN[ i ] = float ( GetInputTrainSet( iPattern, i ) );
    }
    
    NWJNWGT.OWGT = GetEventWeightTrainSet( iPattern );
    
    for( Int_t j = 0; j < outputDim; j++ )
    {
      JNDAT1.OUT[ j ] = float ( GetOutputTrainSet( iPattern, j ) );
    }

    JNTRAL();
  }

  return GetPARJN(8);
}
//______________________________________________________________________________
void JetNet::writeNetworkInfo(Int_t typeOfInfo)
{
  cout << " Invoking info of type: " << typeOfInfo << endl;
  JNSTAT(typeOfInfo);
}  
//______________________________________________________________________________
void JetNet::Init( void )
{
  // Initializes the neuaral network
  Int_t i;
  JNDAT1.MSTJN[ 0 ] = mLayerCount; // Set the number of layers

  // Set the number of nodes for each layer
  for( i = 0; i < mLayerCount; i++ )
  {
    if ( mDebug ) std::cout << "Layer " << i + 1 << " has " << mpLayers[ i ] << " units." << std::endl; 
    JNDAT1.MSTJN[ 9 + i ] = mpLayers[ i ]; 
  }
   
  cout << " calling JNINIT " << endl;
  JNINIT();
  cout << " finishing calling JNINIT " << endl;
  mIsInitialized = kTRUE;
}
//______________________________________________________________________________
Int_t JetNet::Epoch( void )
{
  // Initiate one train/test step the network. 

  Double_t aTrain, aTest;
  if ( mCurrentEpoch < mEpochs )
  {
      mCurrentEpoch++;
      aTrain = Train();

      //      if (mCurrentEpoch%2)

      //      std::cout << " Calls to MSTJN: " << GetMSTJN(6) << 
      //	std::endl;

      if ( mDebug ) 
      {


	std::cout << "[ " << mCurrentEpoch << " ] Train: " << aTrain << std::endl;
      }
      if ( ( mCurrentEpoch % 2 ) == 0 )
      {
	  aTest = Test();
	  //	  if ( mDebug )
	  std::cout << "[" << mCurrentEpoch << "]: " << GetPARJN(8) << " ";
	  std::cout << "Test: " << aTest << std::endl;
      }
  }
  return mCurrentEpoch;
}
//______________________________________________________________________________
void JetNet::SetInputTrainSet( Int_t aPatternInd, Int_t aInputInd, Double_t aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TRAIN set
  mpInputTrainSet->SetData( aPatternInd, aInputInd, aValue );
}
//______________________________________________________________________________
void JetNet::SetOutputTrainSet( Int_t aPatternInd, Int_t aOutputInd, Double_t aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TRAIN set
  mpOutputTrainSet->SetData( aPatternInd, aOutputInd, aValue );
}
//______________________________________________________________________________
void JetNet::SetInputTestSet( Int_t aPatternInd, Int_t aInputInd, Double_t aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TEST set
  mpInputTestSet->SetData( aPatternInd, aInputInd, aValue );
}
//______________________________________________________________________________
Double_t JetNet::GetOutputTrainSet( Int_t aPatternInd, Int_t aOutputInd )
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TRAIN set
  return mpOutputTrainSet->GetData( aPatternInd, aOutputInd );
}
//______________________________________________________________________________
void JetNet::SetEventWeightTrainSet( Int_t aPatternInd, Double_t aValue )
{
  mpInputTrainSet->SetEventWeight(aPatternInd,aValue);
}
//______________________________________________________________________________

void JetNet::SetEventWeightTestSet( Int_t aPatternInd, Double_t aValue )
{
  mpInputTestSet->SetEventWeight(aPatternInd,aValue);
}
//______________________________________________________________________________
Double_t JetNet::GetInputTestSet( Int_t aPatternInd, Int_t aInputInd )
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TEST set
  return mpInputTestSet->GetData( aPatternInd, aInputInd );
}
//______________________________________________________________________________
Double_t JetNet::GetOutputTestSet( Int_t aPatternInd, Int_t aOutputInd )
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TEST set
  return mpOutputTestSet->GetData( aPatternInd, aOutputInd );
}
//______________________________________________________________________________
void  JetNet::SaveDataAscii( TString aFileName )
{
  // Saves the Input/Output test and train data in plain text file
  ofstream out;
  int i, j;

  // Open ASCII file
  out.open( aFileName );

  //Write the number of layers, including the input and output
  out << mLayerCount << std::endl;

  // Write into the file the number of units in input, hidden and output layers  
  for ( i = 0; i < mLayerCount; i++ ) out << mpLayers[ i ] << " ";
  out << std::endl;

  // Write the size of Train and Test sets 
  out << mTrainSetCnt << " " << mTestSetCnt << std::endl;

  // Dump the Train set : Input1 Input2 ... InputN Output1 Output2 ... OutputN
  for ( i = 0; i < mTrainSetCnt; i++ )
  {
    out << GetInputTrainSet( i, 0 );
    for( j = 1; j < mpLayers[ 0 ]; j++ ) out << " " << GetInputTrainSet( i, j );
    for( j = 0; j < mpLayers[ mLayerCount - 1 ]; j++ ) out << " " << GetOutputTrainSet( i, j );
    out << std::endl;
  }

 // Dump the Test set : Input1 Input2 ... InputN Output1 Output2 ... OutputN
  for ( i = 0; i < mTestSetCnt; i++ )
  {
    out << GetInputTestSet( i, 0 );
    for( j = 1; j < mpLayers[ 0 ]; j++ ) out << " " << GetInputTestSet( i, j );
    for( j = 0; j < mpLayers[ mLayerCount - 1 ]; j++ ) out << " " << GetOutputTestSet( i, j );
    out << std::endl;
  }
  // Close the file
  out.close();
}
//______________________________________________________________________________
void  JetNet::LoadDataAscii( TString aFileName )
{
  // Loads the input/output test/train data from plain text file 
  ifstream in;
  int i, j, k, l, m;
  int aiParam[ 5 ];//iTrainCount, iTestCount, iInputDim, iHiddenDim, iOutputDim;
  Bool_t bFlag;
  Double_t tmp;
  Int_t iPatternLength;

  in.open( aFileName );
  bFlag = Bool_t( in.is_open() );
  if ( in )
  { 
    in >> mLayerCount;
    if( mDebug ){ std::cout << "Layers Count Set to " << mLayerCount << std::endl;}
    i = 0;

    delete [] mpLayers;
    mpLayers = new Int_t[ mLayerCount ];

    if( mDebug ){ std::cout << "Updating the Layers Nodes Counters..." << std::endl; }
    while( ( i < mLayerCount ) && ( !in.eof() ) )
    {
      in >> mpLayers[ i ];
      if( mDebug ){ std::cout << "Layer [ " << i + 1 << " ] has " << mpLayers[ i ] << " units" << std::endl; }
      i++;
    }

    mInputDim = mpLayers[ 0 ];
    mOutputDim = mpLayers[ mLayerCount - 1 ];
    mHiddenLayerDim = mLayerCount-2;

    //Get the patterns count per line 
    iPatternLength = mInputDim + mOutputDim;
    if( mDebug ){ std::cout << "Patterns per line = " << iPatternLength << std::endl; } 
    in >> mTrainSetCnt;
    if( mDebug ){ std::cout << "Train Set has " << mTrainSetCnt << " patterns." << std::endl; }
    in >> mTestSetCnt;
    if( mDebug ){ std::cout << "Test Set has " << mTestSetCnt << " patterns." << std::endl; }
    
    delete mpInputTestSet;
    delete mpInputTrainSet;
    delete mpOutputTestSet;
    delete mpOutputTrainSet;
    
    mpInputTrainSet  = new TNeuralDataSet( mTrainSetCnt, GetInputDim() );
    mpInputTestSet   = new TNeuralDataSet( mTestSetCnt, GetInputDim() );
    mpOutputTrainSet = new TNeuralDataSet( mTrainSetCnt, GetOutputDim() );
    mpOutputTestSet  = new TNeuralDataSet( mTestSetCnt, GetOutputDim() );

    i = 0;
    j = 0;
    
    while( ( i < ( mTrainSetCnt + mTestSetCnt ) ) && ( !in.eof() ) )
    {
      j = 0;
      while( ( j < iPatternLength ) && ( !in.eof() ) )
      {
	if( i < mTrainSetCnt )
	{
	  if( j < mInputDim )
	  {
	    //Train Input Set
	    in >> tmp;
	    SetInputTrainSet( i, j, tmp );
	  }
	  else 
	  {
	    //Train Output Set 
	    m = j - mInputDim;
	    in >> tmp;
	    SetOutputTrainSet( i, m, tmp );
	  }
	}
	else
	{
	  l = i - mTrainSetCnt;
	  if( j < mInputDim )
	    {
	      //Test Input Set
	      in >> tmp;
	      SetInputTestSet( l, j, tmp );
	    }
          else
	    {
	      //Test Output Set
	      m = j - mInputDim;
	      in >> tmp;
	      SetOutputTestSet( l, m, tmp );
	    }

	}    
	j++;
      }
      i++;
    }
  }
  in.close();
}
//______________________________________________________________________________
void  JetNet::SaveDataRoot( TString aFileName )
{
  // Saves the neural network in ROOT file
}
//______________________________________________________________________________
void  JetNet::LoadDataRoot( TString aFileName )
{
  // Loads the neural network from ROOT file
}
//______________________________________________________________________________
void JetNet::Evaluate(  )
{
  //evaluates directly the input provided through SetInputs()
  JNTEST();
}
//______________________________________________________________________________
void JetNet::Evaluate( Int_t aPattern  )
{
  // Evaluates the network output form the input data specified by the Test Pattern
  for( Int_t i = 0; i < GetInputDim(); i++ )
  {
	JNDAT1.OIN[ i ] = float ( GetInputTestSet( aPattern, i ) );
  }
  JNTEST();
}
//______________________________________________________________________________
void JetNet::SetInputs( Int_t aIndex, Double_t aValue )
{
  // Directly sets the inputs of the network 
  JNDAT1.OIN[ aIndex ] = float ( aValue );
}
//______________________________________________________________________________
Double_t JetNet::GetOutput( Int_t aIndex )
{
  // Returns the output of the network 
  return Double_t ( JNDAT1.OUT[ aIndex ] );
}
//______________________________________________________________________________
void JetNet::DumpToFile( TString aFileName )
{
  // Dumps the network data into JETNET specific format
  JNDUMP( -8 );
  std::cout << close( 8 ) << std::endl;
  rename( "./fort.8", aFileName );
}
//______________________________________________________________________________
void JetNet::ReadFromFile( TString aFileName )
{
  // Loads the network from JETNET specific file
  rename( aFileName, "./fort.12" );
  JNREAD( -12 );
  Reinitialize();
  rename( "./fort.12", aFileName );
  //std::cout << close( 12 ) << std::endl;
}
//______________________________________________________________________________
Double_t JetNet::GetWeight( Int_t aLayerInd, Int_t aNodeInd, Int_t aConnectedNodeInd ) const
{
  // Returns the node weight in specific Layer
  return Double_t ( JNINT1.W[ JNINDX( aLayerInd, aNodeInd, aConnectedNodeInd )-1 ] );
  //GP: ONE HAS TO PAY ATTENTION TO THIS STUPID -1!!!
}
//______________________________________________________________________________
Double_t JetNet::GetThreshold( Int_t aLayerInd, Int_t aNodeInd) const
{
  //Returns the node threshold in the specific layer
  return Double_t ( JNINT1.T[ JNINDX( aLayerInd, aNodeInd, 0 )-1 ] );
  //GP: ONE HAS TO PAY ATTENTION TO THIS STUPID -1!!!
}
//______________________________________________________________________________
void JetNet::SelectiveFields( Int_t aLayerA, Int_t aNodeA1, Int_t aNodeA2, Int_t aNodeB1, Int_t aNodeB2, Int_t aSwitch )
{
  // JetNet Selective Fields
  Int_t tmp, i1, i2, j1, j2;

  if( ( aLayerA > 0 ) && ( aLayerA < mLayerCount ) )
  {
    i1 = TMath::Abs( aNodeA1 ); 
    i2 = TMath::Abs( aNodeA2 );
    j1 = TMath::Abs( aNodeB1 ); 
    j2 = TMath::Abs( aNodeB2 );

    if( i1 > i2 )
    {
      tmp = i1;
      i1 = i2;
      i2 = i1;
    }//if

    if( i1 > i2 )
    {
      tmp = i1;
      i1 = i2;
      i2 = i1;
    }//if

    if( ( i1 < mpLayers[ aLayerA ] ) && ( i2 < mpLayers[ aLayerA ] ) && 
	( j1 < mpLayers[ aLayerA - 1 ] ) && ( j2 < mpLayers[ aLayerA - 1 ] ) )
    {
      JNSEFI( aLayerA, i1, i2, j1, j2, aSwitch );
    }//if
  } //if
}
//______________________________________________________________________________
void JetNet::Reinitialize( void )
{
  //Initializes the settings of the network
    Int_t i;
    
   mLayerCount = JNDAT1.MSTJN[ 0 ]; // Set the number of layers
   
   delete [] mpLayers;
   mpLayers = new Int_t[ mLayerCount ];
   
  // Set the number of nodes for each layer
  for( i = 0; i < mLayerCount; i++ )
  {
    mpLayers[ i ] = JNDAT1.MSTJN[ 9 + i ]; 
  }
  
  mpInputTrainSet  = new TNeuralDataSet( mTrainSetCnt, GetInputDim() );
  mpInputTestSet   = new TNeuralDataSet( mTestSetCnt, GetInputDim() );
  mpOutputTrainSet = new TNeuralDataSet( mTrainSetCnt, GetOutputDim() );
  mpOutputTestSet  = new TNeuralDataSet( mTestSetCnt, GetOutputDim() );

  mInputDim = mpLayers[ 0 ];
  mOutputDim = mpLayers[ mLayerCount - 1 ];
  mHiddenLayerDim = mLayerCount-2;


}  
//______________________________________________________________________________
void JetNet::Normalize( void )
{
  // Normilizes Inputs (both test and train)
  mpInputTrainSet->Normalize();
  mpInputTestSet->Normalize();
}
//______________________________________________________________________________
void JetNet::Randomize( void )
{
  // Randomizes Inputs and Outputs of both train and test sets
  mpInputTrainSet->Randomize();
  mpInputTestSet->Randomize();
  mpOutputTrainSet->Randomize();
  mpOutputTestSet->Randomize();
}
//______________________________________________________________________________
Int_t JetNet::GetUnitCount( Int_t aLayer )
{ 
  // Returns the number of the units in specfic layer
  if( ( aLayer > -1 ) && ( aLayer < mLayerCount ) ) 
    return JNDAT1.MSTJN[ 9 + aLayer ]; 
}
//______________________________________________________________________________
void JetNet::SetUpdatesPerEpoch( Int_t aValue )
{ 
  // Sets the number of the updates per epoch
  JNDAT1.MSTJN[ 8 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetUpdatingProcedure( Int_t aValue )
{  
  // Set specific weights update function
  JNDAT1.MSTJN[ 4 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetErrorMeasure( Int_t aValue )
{  
  JNDAT1.MSTJN[ 3 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetActivationFunction( Int_t aValue )
{ 
  // Set the kind of activation function used
  JNDAT1.MSTJN[ 2 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetPatternsPerUpdate( Int_t aValue )
{ 
  JNDAT1.MSTJN[ 1 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetLearningRate( Double_t aValue )
{ 
  // Change the Learning Rate
  JNDAT1.PARJN[ 0 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetMomentum( Double_t aValue )
{ 
  JNDAT1.PARJN[ 1 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetInitialWeightsWidth( Double_t aValue )
{ 
  JNDAT1.PARJN[ 3 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetLearningRateDecrease( Double_t aValue )
{ 
  JNDAT1.PARJN[ 10 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
Int_t JetNet::GetUpdatesPerEpoch( void ) const 
{ 
  return JNDAT1.MSTJN[ 8 ]; 
}
//______________________________________________________________________________
Int_t JetNet::GetUpdatingProcedure( void ) const 
{  
  return JNDAT1.MSTJN[ 3 ]; 
}
//______________________________________________________________________________
Int_t JetNet::GetErrorMeasure( void ) const 
{ 
  return JNDAT1.MSTJN[ 3 ]; 
}
//______________________________________________________________________________
Int_t JetNet::GetActivationFunction( void ) const 
{ 
  return JNDAT1.MSTJN[ 2 ]; 
}
//______________________________________________________________________________
Int_t JetNet::GetPatternsPerUpdate( void ) const 
{ 
  return JNDAT1.MSTJN[ 1 ]; 
}
//______________________________________________________________________________
Double_t JetNet::GetLearningRate( void ) const 
{ 
  return JNDAT1.PARJN[ 0 ]; 
}
//______________________________________________________________________________
Double_t JetNet::GetMomentum( void ) const 
{ 
  return JNDAT1.PARJN[ 1 ]; 
}
//______________________________________________________________________________
Double_t JetNet::GetInitialWeightsWidth( void ) const 
{ 
  return JNDAT1.PARJN[ 3 ]; 
}
//______________________________________________________________________________
Double_t JetNet::GetLearningRateDecrease( void ) const 
{ 
  return JNDAT1.PARJN[ 10 ]; 
}
//______________________________________________________________________________
Int_t JetNet::GetMSTJN( Int_t aIndex )
{
  return JNDAT1.MSTJN[ aIndex ]; 
}
//______________________________________________________________________________
Double_t JetNet::GetPARJN( Int_t aIndex )
{
  return JNDAT1.PARJN[ aIndex ];
}
//______________________________________________________________________________
void JetNet::SetMSTJN( Int_t aIndex, Int_t aValue )
{
  JNDAT1.MSTJN[ aIndex ] = aValue;
}
//______________________________________________________________________________
void JetNet::SetPARJN( Int_t aIndex, Double_t aValue )
{
  JNDAT1.PARJN[ aIndex ] = aValue;
}
//______________________________________________________________________________
void JetNet::Shuffle( Bool_t aShuffleTrainSet, Bool_t aShuffleTestSet )
{
  // Shuffles the train and/or test input/output sets
  TTimeStamp ts;
  Int_t Seed = ts.GetSec();
  if ( aShuffleTrainSet )
  {
    
    mpInputTrainSet->Shuffle( Seed );
    mpOutputTrainSet->Shuffle( Seed );
  }
  //Shuffle Test Set
  if ( aShuffleTestSet )
  {
    Seed = ts.GetSec();
    mpInputTestSet->Shuffle( Seed );
    mpOutputTestSet->Shuffle( Seed );
  }

  return;
} 


//EOF
