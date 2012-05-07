//-*-c++-*-
#ifndef __TTrainedNetwork_
#define __TTrainedNetwork_

#include "TObject.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include <math.h>
#include <vector>
#include <string> 
#include <map>

/******************************************************
  @class TTrainedNetwork
  Created : 18-02-2008
  @author Giacinto Piacquadio (giacinto.piacquadio AT physik.uni-freiburg.de)
********************************************************/

class TTrainedNetwork : public TObject
{
public:
  struct Input { 
    std::string name; 
    double offset; 
    double scale; 
  }; 

  typedef std::vector<Double_t> DVec; 
  typedef std::map<std::string, double> DMap; 

  TTrainedNetwork();

  //class takes ownership of all pointers...
  TTrainedNetwork(std::vector<TTrainedNetwork::Input> inputs,
		  Int_t nOutput,
		  std::vector<TVectorD*> & thresholdVectors,
		  std::vector<TMatrixD*> & weightMatrices,
		  Int_t activationFunction = 1,
                  bool linearOutput=false,
                  bool normalizeOutput=false);

  ~TTrainedNetwork();

  std::vector<Input> getInputs() const; 

  void setNewWeights(std::vector<TVectorD*> & thresholdVectors,
		     std::vector<TMatrixD*> & weightMatrices);

  Int_t getnInput() const {return mnInput;};

  Int_t getnHidden() const {return mnHidden;};

  Int_t getnOutput() const {return mnOutput;};

  const std::vector<Int_t> &  getnHiddenLayerSize() const {
    return mnHiddenLayerSize;
  };

  Int_t getActivationFunction() const {return mActivationFunction;};

  const std::vector<TVectorD*> & getThresholdVectors() const {
    return mThresholdVectors;
  };

  const std::vector<TMatrixD*> & weightMatrices() const {
    return mWeightMatrices;
  };

  DVec calculateOutputValues(DVec & input) const;
  // DVec calculateWithNormalization(DVec & input) const; 
  DVec calculateWithNormalization(DMap & input) const;

  bool getIfLinearOutput() const { return mLinearOutput; };

  bool getIfNormalizeOutput() const { return mNormalizeOutput; };

private:

  const static unsigned MAX_LAYER_LENGTH = 1000; 

  Int_t mnInput;
  Int_t mnHidden;
  Int_t mnOutput;

  struct InputNode { 
    int index; 
    double offset; 
    double scale; 
  }; 

  std::map<std::string,InputNode> inputStringToNode; 

  std::vector<Int_t> mnHiddenLayerSize;

  std::vector<TVectorD*> mThresholdVectors;
  std::vector<TMatrixD*> mWeightMatrices;

  Int_t mActivationFunction;

  bool mLinearOutput;

  bool mNormalizeOutput;

  double maxExpValue;

  Double_t sigmoid(Double_t x) const; 

  bool is_consistent() const; 

  ClassDef( TTrainedNetwork, 2 )

};

#endif
