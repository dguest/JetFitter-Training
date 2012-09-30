#include "TNeuralDataSet.h"
#include "TRandom3.h"

ClassImp( TNeuralDataSet )
//______________________________________________________________________________
TNeuralDataSet::TNeuralDataSet( Int_t aNumberOfPatterns, Int_t aNumberOfUnits )
{
  // Default constructor
  mpData = new TMatrixD( aNumberOfPatterns, aNumberOfUnits ); 
  mpWeights = new TVectorD( aNumberOfPatterns);
  for (int i=0;i<mpWeights->GetNrows();i++){
    (*mpWeights)[i]=1.;
  }

  mpNormFactors = new TVectorD( aNumberOfUnits );
  mpShiftFactors = new TVectorD(  aNumberOfUnits);
}
//______________________________________________________________________________
TNeuralDataSet::~TNeuralDataSet()
{
  // Default destructor
  delete mpData;
  delete mpWeights;
  delete mpNormFactors;
  delete mpShiftFactors;
}
//______________________________________________________________________________
void TNeuralDataSet::SetEventWeight( const Int_t aPattern, Double_t aValue )
{
  mpWeights->operator() ( aPattern ) = aValue;
}
//______________________________________________________________________________
void TNeuralDataSet::SetData( const Int_t aPattern, const Int_t aIndex, Double_t aValue )
{
  // Changes the value of cell in the set specified by Pattern number and Unit index
  mpData->operator() ( aPattern, aIndex ) = aValue;
}

//______________________________________________________________________________
void TNeuralDataSet::Shuffle( Int_t aSeed )
{
  // Shuffles data
  TRandom3 Random( aSeed );
  Int_t j;
  Double_t tmp;
  Int_t a = this->GetPatternsCount() - 1;
  for ( Int_t i = 0; i < this->GetPatternsCount(); i++ )
  {
     j = ( Int_t ) ( Random.Rndm() * a );
     for( Int_t p = 0; p < this->GetUnitsCount(); p++ )
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
