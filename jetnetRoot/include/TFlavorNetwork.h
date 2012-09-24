//-*-c++-*-
#ifndef __TFlavorNetwork_
#define __TFlavorNetwork_

#include "TObject.h"
#include "TMatrixD.h"
#include "TVectorD.h"
#include <math.h>
#include <vector>
#include <string> 
#include <map>

/******************************************************
  @class TFlavorNetwork
  Created : 18-02-2008
  @author Giacinto Piacquadio (giacinto.piacquadio AT physik.uni-freiburg.de)
********************************************************/

class TFlavorNetwork : public TObject
{
public:
  struct Input { 
    std::string name; //<- requires unique strings or none at all
    double offset; //<- this value is added to the input before giving to nn
    double scale; //<- after offset is added, input is scaled by this value 
  }; 

  typedef std::vector<Double_t> DVec; 
  typedef std::map<std::string, double> DMap; 
  typedef DMap::const_iterator DMapI; 

  TFlavorNetwork();

  //NOTE: class takes ownership of all pointers.

  // old-school constructor (for compatability)
  TFlavorNetwork(Int_t nInput, 
		 Int_t nHidden, 
		 Int_t nOutput,
		 std::vector<Int_t> & nHiddenLayerSize, 
		 std::vector<TVectorD*> & thresholdVectors,
		 std::vector<TMatrixD*> & weightMatrices,
		 Int_t activationFunction,
		 bool linearOutput = false,
		 bool normalizeOutput = false); 


  // new constructor for normalized nn. 
  // This avoids some chances for logical inconsistency by constructing 
  // the hidden layer size from the thresholdVectors and weightMatrices. 
  // Also runs a consistency check on thresholdVectors and weightMatrices. 
  TFlavorNetwork(std::vector<TFlavorNetwork::Input> inputs,
		 Int_t nOutput,
		 std::vector<TVectorD*> & thresholdVectors,
		 std::vector<TMatrixD*> & weightMatrices,
		 Int_t activationFunction = 1,
		 bool linearOutput=false,
		 bool normalizeOutput=false);

  ~TFlavorNetwork();

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

  // these methods should be optimized. 
  DVec calculateOutputValues(const DVec & input) const;
  DVec calculateWithNormalization(const DVec& input) const;

  // these are intentionally slow: the NN must 
  // rearrange the inputs each time these functions are called. 
  // They are designed for robustness and ease of use, not for highly
  // optimized code. 
  DVec calculateWithNormalization(const DMap & input) const;
  DVec calculateWithNormalization(DMapI begin, DMapI end) const;

  bool getIfLinearOutput() const { return mLinearOutput; };

  bool getIfNormalizeOutput() const { return mNormalizeOutput; };

private:


  Int_t mnInput;
  Int_t mnHidden;
  Int_t mnOutput;

  // in an ideal world these would be one object in a vector, but 
  // storing classes within classes in root is ugly
  std::vector<Double_t> m_input_node_offset; 
  std::vector<Double_t> m_input_node_scale; 
    
  std::map<std::string,int> m_inputStringToNode; 

  std::vector<Int_t> mnHiddenLayerSize;

  std::vector<TVectorD*> mThresholdVectors;
  std::vector<TMatrixD*> mWeightMatrices;

  Int_t mActivationFunction;

  bool mLinearOutput;

  bool mNormalizeOutput;

  double maxExpValue;

  Double_t sigmoid(Double_t x) const; 

  bool is_consistent() const; 

  ClassDef( TFlavorNetwork, 3 )
  
};

#endif
