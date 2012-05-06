#include "TTrainedNetwork.h"
#include <iostream>
#include <limits>
#include <numeric>
#include <cassert>
#include <cstring>

//Since we are single-threaded and never calls outself recursively, we
//can use a global data area to do our work.
//FIXME: We do it like this to avoid adding new member data (because I
//am not 100% sure this class is not persistified somewhere). {Thomas Kittelmann}
#define MAX_LAYER_LENGTH 1000
namespace TTN_internal {
  static double tmpdata[2*MAX_LAYER_LENGTH];
  static double * tmp_array[2] = { &(tmpdata[0]), &(tmpdata[MAX_LAYER_LENGTH]) };
}


TTrainedNetwork::TTrainedNetwork()
{
  mnInput=0;
  mnHidden=0;
  mnOutput=0;
  mActivationFunction=1;
  mLinearOutput=false;
  mNormalizeOutput=false;
  maxExpValue=log(std::numeric_limits<double>::max());
  
}

TTrainedNetwork::TTrainedNetwork(Int_t nInput,
                                 Int_t nHidden,
                                 Int_t nOutput,
                                 std::vector<Int_t> & nHiddenLayerSize,
                                 std::vector<TVectorD*> & thresholdVectors,
                                 std::vector<TMatrixD*> & weightMatrices,
                                 Int_t activationFunction,
                                 bool linearOutput,
                                 bool normalizeOutput)
{
  mnInput=nInput;
  mnHidden=nHidden;
  mnOutput=nOutput;
  mnHiddenLayerSize=nHiddenLayerSize;
  mThresholdVectors=thresholdVectors;
  mWeightMatrices=weightMatrices;
  mActivationFunction=activationFunction;
  mLinearOutput=linearOutput;
  mNormalizeOutput=normalizeOutput;
  maxExpValue=log(std::numeric_limits<double>::max());

  int nlayer_max(mnOutput);
  for (unsigned i=0;i<nHiddenLayerSize.size();++i)
    nlayer_max = std::max<int>(nlayer_max,nHiddenLayerSize[i]);
  if (nlayer_max>=MAX_LAYER_LENGTH) {
    std::cout<<"TTrainedNetwork ERROR Maximal layer size exceeded"<<std::endl;
    assert(false);
  }
}

TTrainedNetwork::~TTrainedNetwork()
{
  std::vector<TVectorD*>::const_iterator vectBegin=mThresholdVectors.begin();
  std::vector<TVectorD*>::const_iterator vectEnd=mThresholdVectors.end();

  for (std::vector<TVectorD*>::const_iterator vectIter=vectBegin;
       vectIter!=vectEnd;
       ++vectIter)
  {
    delete *vectIter;
  }

  std::vector<TMatrixD*>::const_iterator matrixBegin=mWeightMatrices.begin();
  std::vector<TMatrixD*>::const_iterator matrixEnd=mWeightMatrices.end();

  for (std::vector<TMatrixD*>::const_iterator matrixIter=matrixBegin;
       matrixIter!=matrixEnd;
       ++matrixIter)
  {
    delete *matrixIter;
  }

}

void TTrainedNetwork::setNewWeights(std::vector<TVectorD*> & thresholdVectors,
				    std::vector<TMatrixD*> & weightMatrices)
{

  std::vector<TVectorD*>::const_iterator vectBegin=mThresholdVectors.begin();
  std::vector<TVectorD*>::const_iterator vectEnd=mThresholdVectors.end();

  for (std::vector<TVectorD*>::const_iterator vectIter=vectBegin;
       vectIter!=vectEnd;
       ++vectIter)
  {
    delete *vectIter;
  }

  std::vector<TMatrixD*>::const_iterator matrixBegin=mWeightMatrices.begin();
  std::vector<TMatrixD*>::const_iterator matrixEnd=mWeightMatrices.end();

  for (std::vector<TMatrixD*>::const_iterator matrixIter=matrixBegin;
       matrixIter!=matrixEnd;
       ++matrixIter)
  {
    delete *matrixIter;
  }

  mThresholdVectors.clear();
  mWeightMatrices.clear();

  mThresholdVectors=thresholdVectors;
  mWeightMatrices=weightMatrices;

}

std::vector<Double_t>  TTrainedNetwork::calculateOutputValues(std::vector<Double_t> & input) const 
{
  // This method is now highly optimised (apart from the potential use
  // of a cheaper sigmoid function). Please be very careful changing
  // anything here since it is used heavily in reconstruction during
  // Pixel clusterization - Thomas Kittelmann, Oct 2011.

  if (static_cast<int>(input.size()) != mnInput)
  {
    std::cout << "TTrainedNetwork WARNING Input size: " << input.size()
	      << " does not match with network: " << mnInput << std::endl;
    return std::vector<double>();
  }

  const unsigned nTargetLayers(mnHidden+1);
  const unsigned lastTargetLayer(mnHidden);
  unsigned nSource = mnInput, nTarget(0);
  const double * source = &(input[0]);
  double * target(0);
  const double * weights(0);
  const double * thresholds(0);
  double nodeVal(0);

  for (unsigned iLayer = 0; iLayer < nTargetLayers; ++iLayer) {
    //Find data area for target layer:
    nTarget = ( iLayer == lastTargetLayer ? mnOutput : mnHiddenLayerSize[iLayer] );
    target = TTN_internal::tmp_array[iLayer%2];

    //Transfer the input nodes to the output nodes in this layer transition:
    weights = mWeightMatrices[iLayer]->GetMatrixArray();
    thresholds = mThresholdVectors[iLayer]->GetMatrixArray();
    for (unsigned inodeTarget=0;inodeTarget<nTarget;++inodeTarget) {
      nodeVal = 0.0;//Better would be "nodeVal = *thresholds++" and
		    //remove the line further down, but this way we
		    //get exactly the same results that an earlier
		    //version of the package gave.
      const double * weights_tmp = weights++;
      const double * source_end(&(source[nSource]));
      for (const double* source_iter=source;source_iter!=source_end;++source_iter)
	{
	  nodeVal += (*weights_tmp) * (*source_iter);
	  weights_tmp += nTarget;
	}
      nodeVal += *thresholds++;//see remark above.
      target[inodeTarget] = ( mLinearOutput && iLayer == lastTargetLayer )
	                    ? nodeVal : sigmoid(nodeVal);
    }
    //Prepare for next layer transition:
    source = target;
    nSource = nTarget;
  }

  std::vector<double> result(nTarget);
  if (!mNormalizeOutput) {
    std::memcpy(&result[0], target, sizeof(*target)*nTarget);
  } else {
    const double sumLastLayer = std::accumulate(&target[0], &target[nTarget], 0.0 );
    const double normFact = sumLastLayer ? 1.0/sumLastLayer : 0.0;
    for (unsigned i = 0; i < nTarget; ++i)
      result[i] = normFact * target[i];
  }
  
  return result;
}

ClassImp( TTrainedNetwork)




