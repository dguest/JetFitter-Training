// It's -*- c++ -*-
#ifndef __TOldNetwork_
#define __TOldNetwork_

#include "TObject.h"
#include "TMatrixD.h"
#include "TVectorD.h"
// #include "TJetNet.h"
#include <math.h>
#include <vector>

//by Giacinto Piacquadio (18-02-2008)

class TOldNetwork : public TObject
{
 public:
  
  TOldNetwork();

  //class takes ownership of all pointers...
  TOldNetwork(Int_t nInput,
		  Int_t nHidden,
		  Int_t nOutput,
		  std::vector<Int_t> & nHiddenLayerSize,
		  std::vector<TVectorD*> & thresholdVectors,
		  std::vector<TMatrixD*> & weightMatrices,
		  Int_t activationFunction);

  ~TOldNetwork();

  void setNewWeights(std::vector<TVectorD*> & thresholdVectors,
		     std::vector<TMatrixD*> & weightMatrices);

  Int_t getnInput() const {return mnInput;};

  Int_t getnHidden() const {return mnHidden;};

  Int_t getnOutput() const {return mnOutput;};

  const std::vector<Int_t> &  getnHiddenLayerSize() const {return mnHiddenLayerSize;};

  Int_t getActivationFunction() const {return mActivationFunction;};

  const std::vector<TVectorD*> & getThresholdVectors() const {return mThresholdVectors;};

  const std::vector<TMatrixD*> & weightMatrices() const {return mWeightMatrices;};

  std::vector<Double_t> calculateOutputValues(std::vector<Double_t> & input) const;

 private:

  Int_t mnInput;
  Int_t mnHidden;
  Int_t mnOutput;

  std::vector<Int_t> mnHiddenLayerSize;
  //Int_t* mnHiddenLayerSize;

  std::vector<TVectorD*> mThresholdVectors;
  std::vector<TMatrixD*> mWeightMatrices;
  //  TVectorD** mThresholdVectors;
  //  TMatrixD** mWeightMatrices;
  Int_t mActivationFunction;

  Double_t sigmoid(Double_t x) const { return 1./(1.+exp(-2*x)); };

  ClassDef( TOldNetwork, 1 )

};

#endif
