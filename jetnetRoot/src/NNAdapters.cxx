#include <vector>
#include <stdexcept>
#include <cassert>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "JetNet.hh"
#include "TNeuralNetwork.h"
#include "TOldNetwork.h"
#include "NNAdapters.hh"
#include "TVector.h"
#include "TMatrix.h"
#include "TTree.h"
#include "TFile.h"

//by Giacinto Piacquadio (18-02-2008)
TNeuralNetwork* getTrainedNetwork(const JetNet& jn) 
{
  Int_t nInput = jn.GetInputDim();
  Int_t nHidden = jn.GetHiddenLayerDim();
  std::vector<Int_t> nHiddenLayerSize;

  for (Int_t o=0;o<nHidden;++o)
  {
    nHiddenLayerSize.push_back(jn.GetHiddenLayerSize(o+1));
  }
  Int_t nOutput = jn.GetOutputDim();

  std::vector<TVectorD*> thresholdVectors;
  std::vector<TMatrixD*> weightMatrices;

  for (Int_t o=0;o<nHidden+1;++o)
  {
     int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;
     int sizePreviousLayer=(o==0)?nInput:nHiddenLayerSize[o-1];
     thresholdVectors.push_back(new TVectorD(sizeActualLayer));
     weightMatrices.push_back(new TMatrixD(sizePreviousLayer,sizeActualLayer));
  }

  for (Int_t o=0;o<nHidden+1;++o){
    
    int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;

    for (Int_t s=0;s<sizeActualLayer;++s){
      if (o==0) {
	for (Int_t p=0;p<nInput;++p) {
	  (*weightMatrices[o])(p,s) = jn.GetWeight(o+1,s+1,p+1);
        }
      }
      else {
	for (Int_t p=0;p<nHiddenLayerSize[o-1];++p) {
	  (*weightMatrices[o])(p,s) = jn.GetWeight(o+1,s+1,p+1);
	}
      }
      (*thresholdVectors[o])(s) = jn.GetThreshold(o+1,s+1);
    }
  }

  typedef std::vector<JetNet::InputNode> JetNetInputs; 
  JetNetInputs jn_inputs = jn.getInputNodes(); 
  std::vector<TNeuralNetwork::Input> nn_inputs; 
  for (JetNetInputs::const_iterator itr = jn_inputs.begin(); 
       itr != jn_inputs.end(); itr++) { 
    TNeuralNetwork::Input the_node = 
      convert_node<TNeuralNetwork::Input>(*itr); 
    nn_inputs.push_back(the_node); 
  }
       
  int activation_function = jn.GetActivationFunction(); 

  TNeuralNetwork* trainedNetwork=
    new TNeuralNetwork(nn_inputs, 
			nOutput,
			thresholdVectors,
			weightMatrices,
			activation_function);
						
  return trainedNetwork;

}
//___________________________________________________________________________
//by Giacinto Piacquadio (18-02-2008)
void setTrainedNetwork(JetNet& jn, const TNeuralNetwork* trainedNetwork)
{

  Int_t nInput = jn.GetInputDim();
  Int_t nHidden = jn.GetHiddenLayerDim();
  std::vector<Int_t> nHiddenLayerSize;

  int tr_n_hidden = trainedNetwork->getnHidden(); 

  if (tr_n_hidden != nHidden)
  {
    std::string e_string = 
      "Hidden layers mismatch -- JetNet: %i, TNeuralNetwork: %i"; 
    std::string err = (boost::format(e_string) % nHidden % tr_n_hidden).str(); 
    throw std::runtime_error(err); 
  }

  for (Int_t o=0;o<nHidden;++o)
  {
    nHiddenLayerSize.push_back(jn.GetHiddenLayerSize(o+1));
    if (nHiddenLayerSize[o]!=trainedNetwork->getnHiddenLayerSize()[o])
    {
      std::string e_string = "Network doesn't match layer %i ... not loading";
      std::string err = ( boost::format(e_string) % o).str(); 
      throw std::runtime_error(err); 
    }
  }
  Int_t nOutput=jn.GetOutputDim();

  if (trainedNetwork->getnInput()!=nInput)
  {
    throw std::runtime_error(" Network doesn't match nInput"); 
  }


  if (trainedNetwork->getnOutput()!=nOutput)
  {
    throw std::runtime_error(" Network doesn't match nOutput"); 
  }
  
  //OK, everything matches... can go on...
  
  std::vector<TVectorD*> thresholdVectors=trainedNetwork->getThresholdVectors();
  std::vector<TMatrixD*> weightMatrices=trainedNetwork->weightMatrices();
  //ownership remains of the TNeuralNetwork

  for (Int_t o=0;o<nHidden+1;++o)
  {
    int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;
    int sizePreviousLayer=(o==0)?nInput:nHiddenLayerSize[o-1];
  
    for (Int_t s=0;s<sizeActualLayer;++s)
    {
      Double_t nodeValue=0.;
      if (o==0)
      {
	for (Int_t p=0;p<nInput;++p)
	{
	  jn.SetWeight((*weightMatrices[o])(p,s),o+1,s+1,p+1);
        }
      }
      else
      {
	for (Int_t p=0;p<nHiddenLayerSize[o-1];++p)
	{
	  jn.SetWeight((*weightMatrices[o])(p,s),o+1,s+1,p+1);
	}
      }
      jn.SetThreshold((*thresholdVectors[o])(s),o+1,s+1);
    }
  }      

  typedef std::vector<TNeuralNetwork::Input> TrainedInputs; 
  std::vector<JetNet::InputNode> jetnet_inputs; 
  TrainedInputs inputs = trainedNetwork->getInputs(); 
  for (TrainedInputs::const_iterator itr = inputs.begin(); 
       itr != inputs.end(); itr++){ 
    JetNet::InputNode node = convert_node<JetNet::InputNode>(*itr); 
    jetnet_inputs.push_back(node); 
  }

  jn.setInputNodes(jetnet_inputs); 
  
}

// -----------------------------------------------
// converter from old TOldNetwork

TNeuralNetwork* getOldTrainedNetwork(std::string file_name) { 

  TFile file(file_name.c_str()); 
  TOldNetwork* trained = dynamic_cast<TOldNetwork*>
    (file.Get("TOldNetwork")); 
  if (!trained) throw std::runtime_error("missing trained network"); 
  
  int n_input = trained->getnInput(); 

  TTree* normalization = dynamic_cast<TTree*>
    (file.Get("normalization_info")); 
  if (!trained) throw std::runtime_error("missing normalization info"); 
  assert(normalization->GetEntries() == n_input); 

  std::string* name_ptr = new std::string; 
  float offset; 
  float scale; 

  if (normalization->SetBranchAddress("name", &name_ptr) || 
      normalization->SetBranchAddress("offset", &offset) || 
      normalization->SetBranchAddress("scale", &scale) ) { 
    throw std::runtime_error("missing branch"); 
  }

  std::vector<TNeuralNetwork::Input> flav_inputs; 
  for (int n = 0; n < n_input; n++) { 
    normalization->GetEntry(n); 
    TNeuralNetwork::Input input; 
    input.name = *name_ptr; 
    input.offset = offset; 
    input.scale = scale; 
    flav_inputs.push_back(input); 
  }

  delete name_ptr; 

  return convertOldToNew(trained, flav_inputs); 
}

TNeuralNetwork* 
convertOldToNew(const TOldNetwork* trained, 
		std::vector<TNeuralNetwork::Input> flav_inputs)
{
  
  int n_input = trained->getnInput(); 
  int n_hidden = trained->getnHidden(); 
  int n_output = trained->getnOutput(); 
  std::vector<int> hidden_layer_sizes = trained->getnHiddenLayerSize(); 
  std::vector<TVectorD*> threshold_vectors = trained->getThresholdVectors(); 
  std::vector<TMatrixD*> weight_matrices = trained->weightMatrices(); 

  gROOT->cd(); 
  std::vector<TVectorD*> new_threshold_vectors;
  std::vector<TMatrixD*> new_weight_matrices;
  assert(threshold_vectors.size() == weight_matrices.size()); 
  for (int i = 0; i < threshold_vectors.size(); i++) { 
    new_threshold_vectors.push_back
      (dynamic_cast<TVectorD*>(threshold_vectors.at(i)->Clone())); 
    new_weight_matrices.push_back
      (dynamic_cast<TMatrixD*>(weight_matrices.at(i)->Clone())); 
  }

  
  TNeuralNetwork* flav_network = new TNeuralNetwork
    (flav_inputs, 
     n_output, 
     new_threshold_vectors, 
     new_weight_matrices); 
  return flav_network; 

}

TOldNetwork* convertNewToOld(const TNeuralNetwork* trained) { 
  int n_input = trained->getnInput(); 
  int n_hidden = trained->getnHidden(); 
  int n_output = trained->getnOutput(); 
  std::vector<int> hidden_layer_sizes = trained->getnHiddenLayerSize(); 
  std::vector<TVectorD*> threshold_vectors = trained->getThresholdVectors(); 
  std::vector<TMatrixD*> weight_matrices = trained->weightMatrices(); 
  int act_func = trained->getActivationFunction(); 

  gROOT->cd(); 
  std::vector<TVectorD*> new_threshold_vectors;
  std::vector<TMatrixD*> new_weight_matrices;
  assert(threshold_vectors.size() == weight_matrices.size()); 
  for (int i = 0; i < threshold_vectors.size(); i++) { 
    new_threshold_vectors.push_back
      (dynamic_cast<TVectorD*>(threshold_vectors.at(i)->Clone())); 
    new_weight_matrices.push_back
      (dynamic_cast<TMatrixD*>(weight_matrices.at(i)->Clone())); 
  }
  
  TOldNetwork* out_net = new TOldNetwork
    (n_input, 
     n_hidden, 
     n_output, 
     hidden_layer_sizes, 
     threshold_vectors, 
     weight_matrices, 
     act_func); 
  return out_net; 
}
