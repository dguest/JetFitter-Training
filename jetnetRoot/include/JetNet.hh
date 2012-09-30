// this be -*- c++ -*-
/* 
 ROOT Interface to JETNET
 Author: Vassil Verguilov
 Created: 2005.02.22 14:07 EET
 Revisited and extended by: Giacinto Piacquadio (Freiburg University)
 (ATLAS - 18-02-2008)
*/



#ifndef JETNET
#define JETNET

//______________________________________________________________________________
//
// JetNet
//
// This class is not a ROOT wrapper for jetnet library.
//
//______________________________________________________________________________
//

//#define _DEBUG

#include "NeuralDataSet.h"

#include <vector>
#include <string>

//typedef ActivationFunction  TActivationFunction;

class JetNet //: public TObject
{
public:

  struct InputNode { 
    std::string name; 
    double offset; 
    double scale; 
  }; 
  
  JetNet( void );

  JetNet( int aTestCount, int aTrainCount, const int aLayersCnt, const int* aLayers );

  virtual ~JetNet( void );
  
  void Print( void );
  
  int GetTrainSetCnt( void ) const { return mTrainSetCnt; };
  int GetTestSetCnt( void ) const { return mTestSetCnt; };
  int GetInputDim( void ) const { return mpLayers[ 0 ]; };
  int GetHiddenLayerDim( void ) const { return mHiddenLayerDim; };
  int GetHiddenLayerSize(int number) const { return mpLayers[ number ]; };
  int GetOutputDim( void ) const { return mpLayers[ mLayerCount - 1 ]; };

  void SetInputTrainSet( int aPatternInd, int aInputInd, double aValue );
  void SetOutputTrainSet( int aPatternInd, int aOutputInd, double aValue );
  void SetInputTestSet( int aPatternInd, int aInputInd, double aValue );
  void SetOutputTestSet( int aPatternInd, int aOutputInd, double aValue );
  
  void SetEventWeightTrainSet( int aPatternInd, double aValue );
  void SetEventWeightTestSet( int aPatternInd, double aValue );

  double GetInputTrainSet( int aPatternInd, int aInputInd ) const ;
  double GetOutputTrainSet( int aPatternInd, int aOutputInd ) const ;
  double GetInputTestSet( int aPatternInd, int aInputInd ) const ;
  double GetOutputTestSet( int aPatternInd, int aOutputInd ) const;
  
  double GetEventWeightTrainSet( int aPatternInd ) const;
  double GetEventWeightTestSet( int aPatternInd ) const;

  double GetWeight( int aLayerInd, int aNodeInd, 
		      int aConnectedNodeInd ) const; 
  double GetThreshold( int aLayerInd, int aNodeInd) const;

  int GetEpochs( void ) const { return mEpochs; } ;
  void SetEpochs( const int aEpochs ) { mEpochs = aEpochs; mCurrentEpoch = 0; };
  void Init( void );

  double Train( void );
  int Epoch( void );
  double Test( void );

  void Shuffle ( bool aShuffleTrainSet = true, bool aShuffleTestSet = true );
 
  void SaveDataAscii( const char* aFileName = "jndata.dat" ) const;
  void SaveDataRoot( const char* aFileName = "jndata.root" );

  void LoadDataAscii( const char* aFileName = "jndata.dat" );
  void LoadDataRoot( const char* aFileName = "jndata.root" );
  
  void DumpToFile( const char* aFileName = "fort.8" ) const;
  void ReadFromFile( const char* aFileName = "fort.8" );

  double GetOutput( int aIndex = 0 ) const;
  void SetInputs( int aIndex = 0, double aValue = 0.0 );
  void Evaluate( int aPattern );
  void Evaluate();

  void writeNetworkInfo(int typeOfInfo = 0);

  int GetLayerCount( void ) const { return mLayerCount; };
  //Number of layers in NN
  int GetUnitCount( int aLayer ) const ;
  //Number of units in NN  

  void SelectiveFields( int aLayerA, int aNodeA1, int aNodeA2, int aNodeB1, int aNodeB2, int aSwitch = 0 );
  
  void SetUpdatesPerEpoch( int aValue );
  void SetUpdatingProcedure( int aValue );
  void SetErrorMeasure( int aValue );
  void SetActivationFunction( int aValue );
  void SetPatternsPerUpdate( int aValue );
  void SetLearningRate( double aValue );
  void SetMomentum( double aValue );
  void SetInitialWeightsWidth( double aValue );
  void SetLearningRateDecrease( double aValue );


  int GetUpdatesPerEpoch( void ) const ;
  int GetUpdatingProcedure( void ) const ;
  int GetErrorMeasure( void ) const ;
  int GetActivationFunction( void ) const ;
  int GetPatternsPerUpdate( void ) const ;
  double GetLearningRate( void ) const ;
  double GetMomentum( void ) const ;
  double GetInitialWeightsWidth( void ) const ;
  double GetLearningRateDecrease( void ) const ;
    
  void LockInit( void ){ mInitLocked = true; };
  void UnlockInit( void ){ mInitLocked = false; };
  int GetMSTJN( int aIndex ) const;
  double GetPARJN( int aIndex ) const;
  void SetMSTJN( int aIndex, int aValue );
  void SetPARJN( int aIndex, double aValue );

  void setInputNodes(std::vector<InputNode> ); 
  std::vector<InputNode> getInputNodes() const; 

  enum TActivationFunction {
    afSigmoid = 1,
    afTanh    = 2,
    afExp     = 3,
    afLinear  = 4,
    afSigmoidEntropy = 5
  };


  void SetWeight( double weight,int aLayerInd, int aNodeInd, int aConnectedNodeInd ); 
  void SetThreshold( double threshold, int aLayerInd, int aNodeInd);
    
private:
  

  int CopyFile( const char* aSrcFile, const char* aDestFile );
  void Reinitialize( void ); // Synchronizing the paramaters of the class object from JETNET parameters

  TActivationFunction menActFunction;

  int  mLayerCount; // Number of Layers (including the input and output)
  int* mpLayers; //! Array which contains the number of units in each layer 

  NeuralDataSet* mpInputTrainSet;
  NeuralDataSet* mpOutputTrainSet;
  NeuralDataSet* mpInputTestSet;
  NeuralDataSet* mpOutputTestSet;

  int mTrainSetCnt, mTestSetCnt; // Size of Train and Test set

  int mInputDim; // Number of the elements in intput layer
  int mHiddenLayerDim; // Number of Hidden Layers
  int mOutputDim; // Number of outputs
  int mEpochs;    // Epochs
  int mCurrentEpoch;    // Current epoch
  bool mDebug; // Debug Flag
  bool mIsInitialized;
  bool mInitLocked;

  std::vector<InputNode> m_input_nodes; 

  //  ClassDef( JetNet, 1 )
}; 

inline void JetNet::SetOutputTestSet( int aPatternInd, int aOutputInd, double aValue )
{
  // Changes the value of the cell corresponding to unit aInputInd in pattern aPatternInd into OUTPUT TEST set
  mpOutputTestSet->SetData( aPatternInd, aOutputInd, aValue );
}
//______________________________________________________________________________
inline double JetNet::GetInputTrainSet( int aPatternInd, 
					  int aInputInd ) const 
{
  // Returns the value of the cell corresponding to unit aInputInd in pattern aPatternInd into INPUT TRAIN set
  return mpInputTrainSet->GetData( aPatternInd, aInputInd );
}
//______________________________________________________________________________
inline double JetNet::GetEventWeightTrainSet( int aPatternInd ) const
{
  return mpInputTrainSet->GetEventWeight( aPatternInd);
}
//______________________________________________________________________________
inline double JetNet::GetEventWeightTestSet( int aPatternInd ) const
{
  return mpInputTestSet->GetEventWeight( aPatternInd);
}

#endif

