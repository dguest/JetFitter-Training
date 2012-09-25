#include <TH1D.h>
#include <TH2D.h>
#include "TFlavorNetwork.h"
#include "NetworkToHistoTool.hh"
#include <cmath>
#include <stdexcept> 
#include <cassert>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

// ClassImp( TNetworkToHistoTool)

std::map<std::string,TH1*> 
NetworkToHistoTool::histsFromNetwork(const TFlavorNetwork* trainedNetwork) 
  const
{

  std::map<std::string,TH1*> outputHistos;


  assert(trainedNetwork->getActivationFunction() == 1); 

  Int_t nInput=trainedNetwork->getnInput();
  std::vector<Int_t> nHiddenLayerSize=trainedNetwork->getnHiddenLayerSize();
  Int_t nHidden=nHiddenLayerSize.size();

  Int_t nOutput=trainedNetwork->getnOutput();
  
  std::vector<TVectorD*> thresholdVectors=trainedNetwork->getThresholdVectors();
  std::vector<TMatrixD*> weightMatrices=trainedNetwork->weightMatrices();

  //LayersInfo

  TH1D* histoLayersInfo=new TH1D("LayersInfo",
                                 "LayersInfo",
                                 nHidden+2,
                                 0,
                                 nHidden+2);

  histoLayersInfo->SetBinContent(1,nInput);

  for (Int_t i=0;i<nHidden;++i)
  {
    histoLayersInfo->SetBinContent(2+i,nHiddenLayerSize[i]);
  }

  histoLayersInfo->SetBinContent(2+nHidden,nOutput);

  outputHistos[histoLayersInfo->GetName()] = histoLayersInfo;

  
  //ThresholdInfo
  for (Int_t i=0;i<nHidden+1;++i)
  {
    std::string threName = (boost::format("Layer%i_thresholds") % i).str();
   
    Int_t layerSize=(i<nHidden)?nHiddenLayerSize[i]:nOutput;
    Int_t previousLayerSize=(i==0)?nInput:nHiddenLayerSize[i-1];
    
    TH1D* histoThreshLayer=new TH1D(threName.c_str(),
                                    threName.c_str(),
                                    layerSize,
                                    0,
                                    layerSize);
    
    for (Int_t s=0;s<layerSize;s++)
    {
      histoThreshLayer->SetBinContent(s+1,(*thresholdVectors[i])(s));
    }

    std::string weightsName = (boost::format("Layer%i_weights") % i).str();
    
    outputHistos[histoThreshLayer->GetName()] = histoThreshLayer;

    TH2D* histoWeightsLayer=new TH2D(weightsName.c_str(),
                                     weightsName.c_str(),
                                     previousLayerSize,
                                     0,
                                     previousLayerSize,
                                     layerSize,
                                     0,
                                     layerSize);
    
    for (Int_t s=0;s<layerSize;s++)
    {
      for (Int_t p=0;p<previousLayerSize;++p)
      {
        histoWeightsLayer->SetBinContent(p+1,s+1,(*weightMatrices[i])(p,s));
      }
    }
    
    outputHistos[histoWeightsLayer->GetName()] = histoWeightsLayer;
    
  }

  typedef TFlavorNetwork::Input Input; 
  std::vector<Input> inputs = trainedNetwork->getInputs(); 
  
  assert(inputs.size() == nInput); 

  TH2D* histoInputs = new TH2D("InputsInfo", "InputsInfo",
			       nInput, 0, 1, 
			       2, 0, 1); 
  
  for (size_t input_n = 0; input_n < nInput; input_n++ ) { 
    Input input = inputs.at(input_n); 
    histoInputs->SetBinContent(input_n + 1, 1, input.offset); 
    histoInputs->SetBinContent(input_n + 1, 2, input.scale); 
    histoInputs->GetXaxis()->SetBinLabel(input_n + 1, input.name.c_str());
  }
  outputHistos[histoInputs->GetName()] = histoInputs; 

  return outputHistos;
  
}


TFlavorNetwork* 
NetworkToHistoTool::networkFromHists(std::map<std::string,TH1*>& inputHistos,
				     unsigned options) const
{

  

  TH1* histoLayersInfo = inputHistos["LayersInfo"]; 

  if (histoLayersInfo==0)
  {
    throw std::runtime_error(" Could not find LayersInfo histogram..."); 
  }


  Int_t nHidden=histoLayersInfo->GetNbinsX()-2;
  Int_t nInput=(Int_t)std::floor(histoLayersInfo->GetBinContent(1)+0.5);

  std::vector<Int_t> nHiddenLayerSize;
  for (Int_t i=0;i<nHidden;++i)
  {
    nHiddenLayerSize.push_back( (Int_t)std::floor(histoLayersInfo->GetBinContent(2+i)+0.5));
  }

  Int_t nOutput=(Int_t)std::floor(histoLayersInfo->GetBinContent(2+nHidden)+0.5);

  
  std::vector<TVectorD*> thresholdVectors;
  std::vector<TMatrixD*> weightMatrices;


  //Reconstruct thresholdInfo
  for (Int_t i=0;i<nHidden+1;++i)
  {
    std::string threName = (boost::format("Layer%i_thresholds") % i).str();

    Int_t layerSize=(i<nHidden)?nHiddenLayerSize[i]:nOutput;
    Int_t previousLayerSize=(i==0)?nInput:nHiddenLayerSize[i-1];

    TVectorD* thresholdVector=new TVectorD(layerSize);
    TMatrixD* weightMatrix=new TMatrixD(previousLayerSize,layerSize);
   
    TH1* histoThreshLayer = inputHistos[threName]; 
    if (!histoThreshLayer)
      throw std::runtime_error("could not find " + threName); 

    for (Int_t s=0;s<layerSize;s++)
    {
      (*thresholdVector)(s) = histoThreshLayer->GetBinContent(s+1);
    }

    std::string weightsName = (boost::format("Layer%i_weights") % i).str();

    TH1* histoWeightsLayer = inputHistos[weightsName]; 
    if (!histoWeightsLayer) 
      throw std::runtime_error("could not find " + weightsName); 
    for (Int_t s=0;s<layerSize;s++)
    {
      for (Int_t p=0;p<previousLayerSize;++p)
      {
        (*weightMatrix)(p,s) = histoWeightsLayer->GetBinContent(p+1,s+1);
      }
    }

    thresholdVectors.push_back(thresholdVector);
    weightMatrices.push_back(weightMatrix);

  }
  
  TH1* histoInputs = inputHistos["InputsInfo"]; 
  std::vector<TFlavorNetwork::Input> inputs; 
  if (!histoInputs) { 
    for (int i = 0 ; i < nInput; i++) { 
      TFlavorNetwork::Input the_input; 
      the_input.offset = 0; 
      the_input.scale = 1; 
      inputs.push_back(the_input); 
    }
  }
  else { 
    for (int i = 0 ; i < nInput; i++) { 
      TFlavorNetwork::Input the_input; 
      the_input.name = histoInputs->GetXaxis()->GetBinLabel(i + 1); 
      the_input.offset = histoInputs->GetBinContent(i + 1, 1); 
      the_input.scale = histoInputs->GetBinContent(i + 1, 2); 
      inputs.push_back(the_input); 
    }
  }
  TFlavorNetwork* trainedNetwork = 
    new TFlavorNetwork(inputs,
		       nOutput,
		       thresholdVectors,
		       weightMatrices, 
		       options);
  return trainedNetwork;
  
}
