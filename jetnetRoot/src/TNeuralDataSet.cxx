#include "TNeuralDataSet.h"
#include <cstdlib> // rand
//______________________________________________________________________________
TNeuralDataSet::TNeuralDataSet( int n_pattern, int n_unit ): 
  m_n_patterns(n_pattern), 
  m_n_units(n_unit)
{
  // Default constructor
  mpData = new double[ n_pattern * n_unit ]; 
  mpWeights = new double[n_pattern];
  for (int i=0; i < n_pattern; i++){
    mpWeights[i]=1.;
  }

}
//______________________________________________________________________________
TNeuralDataSet::~TNeuralDataSet()
{
  // Default destructor
  delete mpData;
  delete mpWeights;
}
//______________________________________________________________________________
void TNeuralDataSet::SetEventWeight( const int aPattern, double aValue )
{
  mpWeights[aPattern] = aValue;
}
//______________________________________________________________________________
void TNeuralDataSet::SetData( const int aPattern, const int aIndex, double aValue )
{
  int global_index = aPattern * m_n_units + aIndex; 
  // Changes the value of cell in the set specified by Pattern number and Unit index
  mpData[global_index] = aValue;
}

//______________________________________________________________________________
void TNeuralDataSet::Shuffle( int seed )
{
  // Shuffles data
  if (seed) { 
    srand(seed); 
  }
  int j;
  double tmp;
  int a = this->GetPatternsCount() - 1;
  for ( int i = 0; i < this->GetPatternsCount(); i++ )
  {
    i = static_cast<int>( (double(rand() ) / RAND_MAX ) * a );
     for( int p = 0; p < this->GetUnitsCount(); p++ )
     {
	tmp = this->GetData( i, p );
	this->SetData( i, p, this->GetData( j, p ) );
	this->SetData( j, p, tmp );
     }
     tmp = this->GetEventWeight(i);
     this->SetEventWeight(i,this->GetEventWeight(j));
     this->SetEventWeight(j,tmp);
  }
}

//EOF
