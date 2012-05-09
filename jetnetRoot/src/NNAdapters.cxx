#include <vector>
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include "JetNet.hh"
#include "TTrainedNetwork.h"
#include "NNAdapters.hh"

//by Giacinto Piacquadio (18-02-2008)
TTrainedNetwork* getTrainedNetwork(const JetNet& jn) 
{
  bool mDebug = true; 
  
  Int_t nInput = jn.GetInputDim();
  Int_t nHidden = jn.GetHiddenLayerDim();
  std::vector<Int_t> nHiddenLayerSize;
  //  Int_t* nHiddenLayerSize=new Int_t[nHidden];

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
    
    // if (mDebug) { 
    //   if (o<nHidden) {
    // 	cout << " Iterating on hidden layer n.: " << o << endl;
    //   }
    //   else {
    // 	cout << " Considering output layer " << endl;
    //   }
    // }    

    int sizeActualLayer=(o<nHidden)?nHiddenLayerSize[o]:nOutput;

    for (Int_t s=0;s<sizeActualLayer;++s){
      // if (o<nHidden){
      // 	if (mDebug)
      // 	  cout << " To hidden node: " << s << endl;
      // }
      // else {
      // 	if (mDebug)
      // 	  cout << " To output node: " << s << endl;
      // }
      if (o==0) {
	for (Int_t p=0;p<nInput;++p) {
	  // if (mDebug)
	  //   cout << " W from inp nod: " << p << "weight: " <<
	  //     jn.GetWeight(o+1,s+1,p+1) << endl;
	  weightMatrices[o]->operator() (p,s) = jn.GetWeight(o+1,s+1,p+1);
        }
      }
      else {
	for (Int_t p=0;p<nHiddenLayerSize[o-1];++p) {
	  // if (mDebug)
	  //   cout << " W from lay : " << o-1 << " nd: " << 
	  //     p << " weight: " <<
	  //     jn.GetWeight(o+1,s+1,p+1) << endl;
	  weightMatrices[o]->operator() (p,s)=jn.GetWeight(o+1,s+1,p+1);
	}
      }
      // if (mDebug)
      // 	cout << " Threshold for node " << s << " : " << 
      // 	  jn.GetThreshold(o+1,s+1) << endl;
      thresholdVectors[o]->operator() (s) = jn.GetThreshold(o+1,s+1);
    }
  }

  typedef std::vector<JetNet::InputNode> JetNetInputs; 
  JetNetInputs jn_inputs = jn.getInputNodes(); 
  std::vector<TTrainedNetwork::Input> nn_inputs; 
  for (JetNetInputs::const_iterator itr = jn_inputs.begin(); 
       itr != jn_inputs.end(); itr++) { 
    TTrainedNetwork::Input the_node = 
      convert_node<TTrainedNetwork::Input>(*itr); 
    nn_inputs.push_back(the_node); 
  }
       
  int activation_function = jn.GetActivationFunction(); 

  TTrainedNetwork* trainedNetwork=
    new TTrainedNetwork(nInput, 
			nHidden, 
			nOutput,
			nHiddenLayerSize, 
			thresholdVectors,
			weightMatrices,
			activation_function);
						
  return trainedNetwork;

}
//______________________________________________________________________________
//by Giacinto Piacquadio (18-02-2008)
void setTrainedNetwork(JetNet& jn, const TTrainedNetwork* trainedNetwork)
{

  Int_t nInput = jn.GetInputDim();
  Int_t nHidden = jn.GetHiddenLayerDim();
  std::vector<Int_t> nHiddenLayerSize;

  if (trainedNetwork->getnHidden()!=nHidden)
  {
    throw std::runtime_error(" Network doesn't match nHidden");
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
  //ownership remains of the TTrainedNetwork

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
	  jn.SetWeight(weightMatrices[o]->operator() (p,s),o+1,s+1,p+1);
        }
      }
      else
      {
	for (Int_t p=0;p<nHiddenLayerSize[o-1];++p)
	{
	  jn.SetWeight(weightMatrices[o]->operator() (p,s),o+1,s+1,p+1);
	}
      }
      jn.SetThreshold(thresholdVectors[o]->operator() (s),o+1,s+1);
    }
  }      

  typedef std::vector<TTrainedNetwork::Input> TrainedInputs; 
  std::vector<JetNet::InputNode> jetnet_inputs; 
  TrainedInputs inputs = trainedNetwork->getInputs(); 
  for (TrainedInputs::const_iterator itr = inputs.begin(); 
       itr != inputs.end(); itr++){ 
    JetNet::InputNode node = convert_node<JetNet::InputNode>(*itr); 
    jetnet_inputs.push_back(node); 
  }

  jn.setInputNodes(jetnet_inputs); 
  
}
