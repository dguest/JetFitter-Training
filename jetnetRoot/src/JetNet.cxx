#include "JetNet.hh"
#include "jetnet.h"
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
 
//Constructors
//______________________________________________________________________________
JetNet::JetNet()
{
  // Default Constructor
  mTestSetCnt = 0;
  mTrainSetCnt = 0;
  mLayerCount = 0;

  mpLayers = 0;

  mIsInitialized = false;
  mInitLocked = false;

  mpInputTrainSet = 0;
  mpInputTestSet = 0;
  mpOutputTrainSet = 0;
  mpOutputTestSet = 0;

  mEpochs = 0;
  mCurrentEpoch = 0;
}
//______________________________________________________________________________
JetNet::JetNet( int aTestCount, int aTrainCount,
	 const int aLayersCnt, const int* aLayers )
{
  // Creates neural network with aLayersCnt number of layers,
  // aTestCount number of patterns for the test set,
  // aTrainCount patterns for the train set.
  // aLayers contains the information for number of the units in the different layers

  mDebug = false;
#ifdef _DEBUG
  mDebug = true;
#endif
  
  if( mDebug ){ std::cout << "=====> Entering JetNet::JetNet(...)" << std::endl; }

  mTestSetCnt  = aTestCount;
  mTrainSetCnt = aTrainCount;
  mLayerCount  = aLayersCnt; // Get the number of layers
  
  if( mLayerCount > 0 )
  {  
   //Perform deep copy of the array holding Layers count
    mpLayers = new int[ mLayerCount ];
    for(int i = 0; i < mLayerCount; ++i )
    {
      mpLayers[ i ] = aLayers[ i ];
    }
  }

  mInputDim = mpLayers[ 0 ];
  mOutputDim = mpLayers[ mLayerCount - 1 ];
  mHiddenLayerDim = mLayerCount-2;
  

  mIsInitialized = false;
  mInitLocked = false;

  mpInputTrainSet  = new NeuralDataSet( mTrainSetCnt, GetInputDim() );
  mpInputTestSet = new NeuralDataSet( mTestSetCnt, GetInputDim() );
  mpOutputTrainSet = new NeuralDataSet( mTrainSetCnt, GetOutputDim() );
  mpOutputTestSet = new NeuralDataSet( mTestSetCnt, GetOutputDim() );
  
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

  
void JetNet::SetWeight( double weight,int aLayerInd, int aNodeInd, int aConnectedNodeInd )
{
  JNINT1.W[ JNINDX( aLayerInd, aNodeInd, aConnectedNodeInd )-1 ]=weight;
}
//______________________________________________________________________________
void JetNet::SetThreshold( double threshold, int aLayerInd, int aNodeInd)
{
  JNINT1.T[ JNINDX( aLayerInd, aNodeInd, 0 )-1 ]=threshold;
}
//______________________________________________________________________________
void JetNet::Print( void )
{
  // Prints on the screen, information for the neural network
  int i;

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
double JetNet::Test( void )
{
  // Initiate test cycle of the neural network
  int NRight = 0;
  double fMeanError = 0.0;
  double *TMP;
  int  NPatterns = GetTestSetCnt();
 

  for( int iPattern = 0; iPattern < NPatterns; iPattern++ )
  {

      for( int i = 0; i < GetInputDim(); i++ )
      {
	JNDAT1.OIN[ i ] = float ( GetInputTestSet( iPattern, i ) );
      }

      NWJNWGT.OWGT = GetEventWeightTestSet( iPattern );

      JNTEST();

      for( int j = 0; j < GetOutputDim(); j++ )
      {
	fMeanError+= NWJNWGT.OWGT * 
	  std::pow(JNDAT1.OUT[ j ]-float( GetOutputTestSet( iPattern, j )),2)/(float)GetOutputDim();
      }



      if( mDebug ) std::cout << "Testing [ " << iPattern << " ] - "  << JNDAT1.OIN[ 0 ] 
			     << " => " << JNDAT1.OUT[ 0 ] << std::endl;

    }

  fMeanError/=2.*NPatterns;

  if (mDebug)
    std::cout << " Test error: " << fMeanError << std::endl;

  return fMeanError;
}
//

//______________________________________________________________________________
double JetNet::Train( void )
{
  // Initiate the train phase for the neural network
  int NRight = 0;
  double fMeanError = 0.0;
  int  NPatterns = GetTrainSetCnt();

  //  cout << " NPatterns is: " << NPatterns << endl;

  int inputDim=GetInputDim();
  int outputDim=GetOutputDim();
  int updatesPerEpoch=GetUpdatesPerEpoch();
  int patternsPerUpdate=GetPatternsPerUpdate();
  
  if (updatesPerEpoch*patternsPerUpdate<1./2.*NPatterns) 
  {
    std::cout << "Using only: " << updatesPerEpoch*patternsPerUpdate << 
      " patterns on available: " << NPatterns << std::endl;
  } else if (updatesPerEpoch*patternsPerUpdate>NPatterns) 
  {
    std::cout << " Trying to use " << updatesPerEpoch*patternsPerUpdate << 
      " patterns, but available: " << NPatterns << std::endl;
    return -100;
  }
  
  for( int iPattern = 0; iPattern < updatesPerEpoch*patternsPerUpdate;
       iPattern++ )
  {
    for( int i = 0; i < inputDim; i++ )
    {
      JNDAT1.OIN[ i ] = float ( GetInputTrainSet( iPattern, i ) );
    }
    
    NWJNWGT.OWGT = GetEventWeightTrainSet( iPattern );
    
    for( int j = 0; j < outputDim; j++ )
    {
      JNDAT1.OUT[ j ] = float ( GetOutputTrainSet( iPattern, j ) );
    }

    JNTRAL();
  }

  return GetPARJN(8);
}
//______________________________________________________________________________
void JetNet::writeNetworkInfo(int typeOfInfo)
{
  std::cout << " Invoking info of type: " << typeOfInfo << std::endl;
  JNSTAT(typeOfInfo);
}  
//______________________________________________________________________________
void JetNet::Init( void )
{
  // Initializes the neuaral network
  int i;
  JNDAT1.MSTJN[ 0 ] = mLayerCount; // Set the number of layers

  // Set the number of nodes for each layer
  for( i = 0; i < mLayerCount; i++ )
  {
    if ( mDebug ) std::cout << "Layer " << i + 1 << " has " << mpLayers[ i ] << " units." << std::endl; 
    JNDAT1.MSTJN[ 9 + i ] = mpLayers[ i ]; 
  }
   
  std::cout << " calling JNINIT " << std::endl;
  JNINIT();
  std::cout << " finishing calling JNINIT " << std::endl;
  mIsInitialized = true;
}
//______________________________________________________________________________
int JetNet::Epoch( void )
{
  // Initiate one train/test step the network. 

  double aTrain, aTest;
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
void JetNet::SetInputTrainSet( int aPatternInd, int aInputInd, double aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TRAIN set
  mpInputTrainSet->SetData( aPatternInd, aInputInd, aValue );
}
//______________________________________________________________________________
void JetNet::SetOutputTrainSet( int aPatternInd, int aOutputInd, double aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TRAIN set
  mpOutputTrainSet->SetData( aPatternInd, aOutputInd, aValue );
}
//______________________________________________________________________________
void JetNet::SetInputTestSet( int aPatternInd, int aInputInd, double aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TEST set
  mpInputTestSet->SetData( aPatternInd, aInputInd, aValue );
}
//______________________________________________________________________________
double JetNet::GetOutputTrainSet( int aPatternInd, 
				    int aOutputInd ) const 
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TRAIN set
  return mpOutputTrainSet->GetData( aPatternInd, aOutputInd );
}
//______________________________________________________________________________
void JetNet::SetEventWeightTrainSet( int aPatternInd, double aValue )
{
  mpInputTrainSet->SetEventWeight(aPatternInd,aValue);
}
//______________________________________________________________________________

void JetNet::SetEventWeightTestSet( int aPatternInd, double aValue )
{
  mpInputTestSet->SetEventWeight(aPatternInd,aValue);
}
//______________________________________________________________________________
double JetNet::GetInputTestSet( int aPatternInd, int aInputInd ) const
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TEST set
  return mpInputTestSet->GetData( aPatternInd, aInputInd );
}
//______________________________________________________________________________
double JetNet::GetOutputTestSet( int aPatternInd, int aOutputInd ) const
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TEST set
  return mpOutputTestSet->GetData( aPatternInd, aOutputInd );
}
//______________________________________________________________________________
void  JetNet::SaveDataAscii( const char* aFileName ) const 
{
  // Saves the Input/Output test and train data in plain text file
  std::ofstream out;
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
void  JetNet::LoadDataAscii( const char* aFileName )
{
  // Loads the input/output test/train data from plain text file 
  std::ifstream in;
  int i, j, k, l, m;
  int aiParam[ 5 ];//iTrainCount, iTestCount, iInputDim, iHiddenDim, iOutputDim;
  bool bFlag;
  double tmp;
  int iPatternLength;

  in.open( aFileName );
  bFlag = bool( in.is_open() );
  if ( in )
  { 
    in >> mLayerCount;
    if( mDebug ){ std::cout << "Layers Count Set to " << mLayerCount << std::endl;}
    i = 0;

    delete [] mpLayers;
    mpLayers = new int[ mLayerCount ];

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
    
    mpInputTrainSet  = new NeuralDataSet( mTrainSetCnt, GetInputDim() );
    mpInputTestSet   = new NeuralDataSet( mTestSetCnt, GetInputDim() );
    mpOutputTrainSet = new NeuralDataSet( mTrainSetCnt, GetOutputDim() );
    mpOutputTestSet  = new NeuralDataSet( mTestSetCnt, GetOutputDim() );

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
void  JetNet::SaveDataRoot( const char* aFileName )
{
  // Saves the neural network in ROOT file
}
//______________________________________________________________________________
void  JetNet::LoadDataRoot( const char* aFileName )
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
void JetNet::Evaluate( int aPattern  )
{
  // Evaluates the network output form the input data specified by the Test Pattern
  for( int i = 0; i < GetInputDim(); i++ )
  {
	JNDAT1.OIN[ i ] = float ( GetInputTestSet( aPattern, i ) );
  }
  JNTEST();
}
//______________________________________________________________________________
void JetNet::SetInputs( int aIndex, double aValue )
{
  // Directly sets the inputs of the network 
  JNDAT1.OIN[ aIndex ] = float ( aValue );
}
//______________________________________________________________________________
double JetNet::GetOutput( int aIndex ) const 
{
  // Returns the output of the network 
  return double ( JNDAT1.OUT[ aIndex ] );
}
//______________________________________________________________________________
void JetNet::DumpToFile( const char* aFileName ) const 
{
  // Dumps the network data into JETNET specific format
  JNDUMP( -8 );
  std::cout << close( 8 ) << std::endl;
  rename( "./fort.8", aFileName );
}
//______________________________________________________________________________
void JetNet::ReadFromFile( const char* aFileName )
{
  // Loads the network from JETNET specific file
  rename( aFileName, "./fort.12" );
  JNREAD( -12 );
  Reinitialize();
  rename( "./fort.12", aFileName );
  //std::cout << close( 12 ) << std::endl;
}
//______________________________________________________________________________
double JetNet::GetWeight( int aLayerInd, int aNodeInd, int aConnectedNodeInd ) const
{
  // Returns the node weight in specific Layer
  return double ( JNINT1.W[ JNINDX( aLayerInd, aNodeInd, aConnectedNodeInd )-1 ] );
  //GP: ONE HAS TO PAY ATTENTION TO THIS STUPID -1!!!
}
//______________________________________________________________________________
double JetNet::GetThreshold( int aLayerInd, int aNodeInd) const
{
  //Returns the node threshold in the specific layer
  return double ( JNINT1.T[ JNINDX( aLayerInd, aNodeInd, 0 )-1 ] );
  //GP: ONE HAS TO PAY ATTENTION TO THIS STUPID -1!!!
}
//______________________________________________________________________________
void JetNet::SelectiveFields( int aLayerA, int aNodeA1, int aNodeA2, int aNodeB1, int aNodeB2, int aSwitch )
{
  // JetNet Selective Fields
  int tmp, i1, i2, j1, j2;

  if( ( aLayerA > 0 ) && ( aLayerA < mLayerCount ) )
  {
    i1 = abs( aNodeA1 ); 
    i2 = abs( aNodeA2 );
    j1 = abs( aNodeB1 ); 
    j2 = abs( aNodeB2 );

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
    int i;
    
   mLayerCount = JNDAT1.MSTJN[ 0 ]; // Set the number of layers
   
   delete [] mpLayers;
   mpLayers = new int[ mLayerCount ];
   
  // Set the number of nodes for each layer
  for( i = 0; i < mLayerCount; i++ )
  {
    mpLayers[ i ] = JNDAT1.MSTJN[ 9 + i ]; 
  }
  
  mpInputTrainSet  = new NeuralDataSet( mTrainSetCnt, GetInputDim() );
  mpInputTestSet   = new NeuralDataSet( mTestSetCnt, GetInputDim() );
  mpOutputTrainSet = new NeuralDataSet( mTrainSetCnt, GetOutputDim() );
  mpOutputTestSet  = new NeuralDataSet( mTestSetCnt, GetOutputDim() );

  mInputDim = mpLayers[ 0 ];
  mOutputDim = mpLayers[ mLayerCount - 1 ];
  mHiddenLayerDim = mLayerCount-2;


}  

//______________________________________________________________________________
int JetNet::GetUnitCount( int aLayer ) const 
{ 
  // Returns the number of the units in specfic layer
  if( ( aLayer > -1 ) && ( aLayer < mLayerCount ) ) 
    return JNDAT1.MSTJN[ 9 + aLayer ]; 
}
//______________________________________________________________________________
void JetNet::SetUpdatesPerEpoch( int aValue )
{ 
  // Sets the number of the updates per epoch
  JNDAT1.MSTJN[ 8 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetUpdatingProcedure( int aValue )
{  
  // Set specific weights update function
  JNDAT1.MSTJN[ 4 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetErrorMeasure( int aValue )
{  
  JNDAT1.MSTJN[ 3 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetActivationFunction( int aValue )
{ 
  // Set the kind of activation function used
  JNDAT1.MSTJN[ 2 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetPatternsPerUpdate( int aValue )
{ 
  JNDAT1.MSTJN[ 1 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetLearningRate( double aValue )
{ 
  // Change the Learning Rate
  JNDAT1.PARJN[ 0 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetMomentum( double aValue )
{ 
  JNDAT1.PARJN[ 1 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetInitialWeightsWidth( double aValue )
{ 
  JNDAT1.PARJN[ 3 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
void JetNet::SetLearningRateDecrease( double aValue )
{ 
  JNDAT1.PARJN[ 10 ] = aValue; 
  // if( !mInitLocked ) this->Init();
}
//______________________________________________________________________________
int JetNet::GetUpdatesPerEpoch( void ) const 
{ 
  return JNDAT1.MSTJN[ 8 ]; 
}
//______________________________________________________________________________
int JetNet::GetUpdatingProcedure( void ) const 
{  
  return JNDAT1.MSTJN[ 3 ]; 
}
//______________________________________________________________________________
int JetNet::GetErrorMeasure( void ) const 
{ 
  return JNDAT1.MSTJN[ 3 ]; 
}
//______________________________________________________________________________
int JetNet::GetActivationFunction( void ) const 
{ 
  return JNDAT1.MSTJN[ 2 ]; 
}
//______________________________________________________________________________
int JetNet::GetPatternsPerUpdate( void ) const 
{ 
  return JNDAT1.MSTJN[ 1 ]; 
}
//______________________________________________________________________________
double JetNet::GetLearningRate( void ) const 
{ 
  return JNDAT1.PARJN[ 0 ]; 
}
//______________________________________________________________________________
double JetNet::GetMomentum( void ) const 
{ 
  return JNDAT1.PARJN[ 1 ]; 
}
//______________________________________________________________________________
double JetNet::GetInitialWeightsWidth( void ) const 
{ 
  return JNDAT1.PARJN[ 3 ]; 
}
//______________________________________________________________________________
double JetNet::GetLearningRateDecrease( void ) const 
{ 
  return JNDAT1.PARJN[ 10 ]; 
}
//______________________________________________________________________________
int JetNet::GetMSTJN( int aIndex ) const 
{
  return JNDAT1.MSTJN[ aIndex ]; 
}
//______________________________________________________________________________
double JetNet::GetPARJN( int aIndex ) const 
{
  return JNDAT1.PARJN[ aIndex ];
}
//______________________________________________________________________________
void JetNet::SetMSTJN( int aIndex, int aValue )
{
  JNDAT1.MSTJN[ aIndex ] = aValue;
}
//______________________________________________________________________________
void JetNet::SetPARJN( int aIndex, double aValue )
{
  JNDAT1.PARJN[ aIndex ] = aValue;
}
//______________________________________________________________________________
void JetNet::Shuffle( bool aShuffleTrainSet, bool aShuffleTestSet )
{
  // Shuffles the train and/or test input/output sets
  int Seed = time(0); 
  if ( aShuffleTrainSet )
  {
    
    mpInputTrainSet->Shuffle( Seed );
    mpOutputTrainSet->Shuffle( Seed );
  }
  //Shuffle Test Set
  if ( aShuffleTestSet )
  {
    Seed = time(0); 
    mpInputTestSet->Shuffle( Seed );
    mpOutputTestSet->Shuffle( Seed );
  }

  return;
} 


//EOF
