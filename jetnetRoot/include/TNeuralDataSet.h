// -*-c++-*-
// Author: Vassil Verguilov  11/05/2006
// Changed a bit by Dan Guest (2012)

#ifndef NEURAL_DATASET
#define NEURAL_DATASET

//______________________________________________________________________________
//
// TNeuralData
//
// This class is the base class for Neural Network Input/Output patterns
// It contains methods for manupulating the data such as Randomize(),
// Normalize(), Shuffle() and others.
//
//______________________________________________________________________________
//

class TNeuralDataSet
{
 public:
  TNeuralDataSet( int aNumberOfPatterns = 0, int aNumberOfUnits = 0 );
  virtual ~TNeuralDataSet( void );
  
  // Returns the number of the patterns in set
  int GetPatternsCount( void ){ return m_n_patterns; };
  // Returns the number of the units in pattern
  int GetUnitsCount( void ){ return m_n_units; };
  // Returns the data in cell defined by pattern number and unit index in the pattern
  double GetData( const int aPattern, const int aIndex );
  // Change the data in cell defined by pattern number and unit index in the pattern
  void     SetData( const int aPattern, const int aIndex, double aValue );

  double GetEventWeight( const int aPattern );
  void SetEventWeight( const int aPattern, double aValue );

  // Shuffles the patterns
  void     Shuffle( int aSeed = 0 );

 private:  

  int m_n_patterns; 
  int m_n_units; 
  //data matrix
  double* mpData;

  //vector (dim= entries)
  double* mpWeights;
};

inline double TNeuralDataSet::GetData( const int aPattern, const int aIndex )
{
  // Returns the value of cell in the set specified by Pattern number and Unit index
  int global_index = aPattern * m_n_units + aIndex; 
  return mpData[global_index];
}
//______________________________________________________________________________
inline double TNeuralDataSet::GetEventWeight( const int aPattern )
{
  return mpWeights[aPattern];
}
//______________________________________________________________________________



#endif
