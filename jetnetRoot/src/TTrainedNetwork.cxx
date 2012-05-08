#include "TTrainedNetwork.h"
#include <iostream>
#include <limits>
#include <numeric>
#include <cassert>
#include <cstring>

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
  mnInput = nInput; 
  mnHidden = nHidden; 
  mnOutput = nOutput;
  mnHiddenLayerSize = nHiddenLayerSize; 
  mThresholdVectors = thresholdVectors;
  mWeightMatrices = weightMatrices;
  mActivationFunction = activationFunction;
  mLinearOutput = linearOutput;
  mNormalizeOutput = normalizeOutput;
  maxExpValue = log(std::numeric_limits<double>::max());
  
}

TTrainedNetwork::TTrainedNetwork(std::vector<TTrainedNetwork::Input> inputs, 
                                 Int_t nOutput,
                                 std::vector<TVectorD*> & thresholdVectors,
                                 std::vector<TMatrixD*> & weightMatrices,
                                 Int_t activationFunction,
                                 bool linearOutput,
                                 bool normalizeOutput)
{
  mnInput = inputs.size(); 
  mnHidden = thresholdVectors.size() - 1;
  mnOutput = nOutput;
  mThresholdVectors = thresholdVectors;
  mWeightMatrices = weightMatrices;
  mActivationFunction = activationFunction;
  mLinearOutput = linearOutput;
  mNormalizeOutput = normalizeOutput;
  maxExpValue = log(std::numeric_limits<double>::max());

  for (std::vector<TVectorD*>::const_iterator tr_itr 
	 = mThresholdVectors.begin(); 
       tr_itr != mThresholdVectors.end(); 
       tr_itr++){ 
    mnHiddenLayerSize.push_back((*tr_itr)->GetNrows()); 
  }

  int node_n = 0; 
  for (std::vector<Input>::const_iterator itr = inputs.begin(); 
       itr != inputs.end(); 
       itr++) { 
    m_input_node_offset.push_back(itr->offset); 
    m_input_node_scale.push_back(itr->scale); 
    inputStringToNode[itr->name] = node_n; 
    node_n++; 
  }

  int n_node = node_n; 
  assert(n_node == inputStringToNode.size()); 
  assert(n_node == m_input_node_offset.size()); 
  assert(n_node == m_input_node_scale.size()); 

  int nlayer_max(mnOutput);
  for (unsigned i = 0; i < mnHiddenLayerSize.size(); ++i)
    nlayer_max = std::max(nlayer_max, mnHiddenLayerSize[i]);
  if (nlayer_max>=MAX_LAYER_LENGTH) {
    std::cout<<"TTrainedNetwork ERROR Maximal layer size exceeded"<<std::endl;
    assert(false);
  }

  assert(is_consistent()); 
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

std::vector<TTrainedNetwork::Input> TTrainedNetwork::getInputs() const { 
  std::map<int,Input> inputs; 
  for (std::map<std::string,int>::const_iterator itr = 
	 inputStringToNode.begin(); 
       itr != inputStringToNode.end(); 
       itr++){ 
    Input the_input; 
    the_input.name = itr->first; 
    the_input.offset = m_input_node_offset.at(itr->second); 
    the_input.scale = m_input_node_scale.at(itr->second);
    inputs[itr->second] = the_input; 
  }
  assert(inputs.size() == inputStringToNode.size()); 

  std::vector<Input> inputs_vector; 
  for (std::map<int,Input>::const_iterator itr = inputs.begin(); 
       itr != inputs.end(); itr++){ 
    inputs_vector.push_back(itr->second); 
  }
  return inputs_vector; 
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

std::vector<Double_t> 
TTrainedNetwork::calculateWithNormalization(TTrainedNetwork::DMap& in) const { 
  std::vector<Double_t> raw_inputs(mnInput); 
  for (std::map<std::string,double>::const_iterator itr = in.begin(); 
       itr != in.end(); 
       itr++){ 
    std::map<std::string,int>::const_iterator input_node_ptr = 
      inputStringToNode.find(itr->first); 
    assert(input_node_ptr != inputStringToNode.end()); 
    const int node_n = input_node_ptr->second; 

    // get and scale the raw input value
    double raw_value = itr->second; 
    raw_value += m_input_node_offset.at(node_n); 
    raw_value *= m_input_node_scale.at(node_n); 

    // store in the inputs vector
    raw_inputs.at(node_n) = raw_value; 
  }
  return calculateOutputValues(raw_inputs); 
}

std::vector<Double_t>  
TTrainedNetwork::calculateOutputValues(std::vector<Double_t> & input) const 
{
  // This method is now highly optimised (apart from the potential use
  // of a cheaper sigmoid function). Please be very careful changing
  // anything here since it is used heavily in reconstruction during
  // Pixel clusterization - Thomas Kittelmann, Oct 2011.

  // I changed someting: these arrays (tmpdata, tmp_array) were global. 
  // This may be slower, but we don't need the speed (and thread safety 
  // is good )  -- Dan Guest, May 2012

  double tmpdata[2*MAX_LAYER_LENGTH];
  double * tmp_array[2] = { 
    &(tmpdata[0]), 
    &(tmpdata[MAX_LAYER_LENGTH]) 
  };


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
    nTarget = ( iLayer == lastTargetLayer ? 
		mnOutput : 
		mnHiddenLayerSize[iLayer] );
    target = tmp_array[iLayer%2];

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
      for (const double* source_iter = source; 
	   source_iter != source_end; ++source_iter)
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
    const double sumLastLayer = 
      std::accumulate(&target[0], &target[nTarget], 0.0 );
    const double normFact = sumLastLayer ? 1.0/sumLastLayer : 0.0;
    for (unsigned i = 0; i < nTarget; ++i)
      result[i] = normFact * target[i];
  }
  
  return result;
}


Double_t TTrainedNetwork::sigmoid(Double_t x) const { 
  if (-2*x >= maxExpValue){
    return 0.;
  }
  return 1./(1.+exp(-2*x)); 
}

bool TTrainedNetwork::is_consistent() const { 
  if (mThresholdVectors.size() != mWeightMatrices.size()) 
    return false; 
  for (unsigned layer_n = 0; layer_n < mThresholdVectors.size(); layer_n++){ 
    int n_threshold_nodes = mThresholdVectors.at(layer_n)->GetNrows(); 
    int n_weights_nodes = mWeightMatrices.at(layer_n)->GetNcols(); 
    if (n_threshold_nodes != n_weights_nodes) return false; 
  }
  return true; 
}

ClassImp( TTrainedNetwork)



