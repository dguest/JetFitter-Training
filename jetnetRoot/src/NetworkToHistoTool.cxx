#include <TH1F.h>
#include <TH2F.h>
#include "TFlavorNetwork.h"
#include "NetworkToHistoTool.hh"
#include <cmath>
#include <stdexcept> 
#include <cassert>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

// ClassImp( TNetworkToHistoTool)

std::vector<TH1*> 
NetworkToHistoTool::fromTrainedNetworkToHisto(TFlavorNetwork* trainedNetwork) const
{

  std::vector<TH1*> outputHistos;


  assert(trainedNetwork->getActivationFunction() == 1); 

  Int_t nInput=trainedNetwork->getnInput();
  std::vector<Int_t> nHiddenLayerSize=trainedNetwork->getnHiddenLayerSize();
  Int_t nHidden=nHiddenLayerSize.size();

  // for (Int_t o=0;o<nHidden;++o)
  // {
  //   cout << " Hidden lay: " << o << " size: " << nHiddenLayerSize[o];
  // }
  
  Int_t nOutput=trainedNetwork->getnOutput();
  // cout << " Output size: " << nOutput << endl;
  
  std::vector<TVectorD*> thresholdVectors=trainedNetwork->getThresholdVectors();
  std::vector<TMatrixD*> weightMatrices=trainedNetwork->weightMatrices();

  //LayersInfo

  TH1F* histoLayersInfo=new TH1F("LayersInfo",
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

  outputHistos.push_back(histoLayersInfo);

  
  //ThresholdInfo
  for (Int_t i=0;i<nHidden+1;++i)
  {
    std::string threName = (boost::format("Layer%i_thresholds") % i).str();
   
    Int_t layerSize=(i<nHidden)?nHiddenLayerSize[i]:nOutput;
    Int_t previousLayerSize=(i==0)?nInput:nHiddenLayerSize[i-1];
    
    TH1F* histoThreshLayer=new TH1F(threName.c_str(),
                                    threName.c_str(),
                                    layerSize,
                                    0,
                                    layerSize);
    
    for (Int_t s=0;s<layerSize;s++)
    {
      histoThreshLayer->SetBinContent(s+1,(*thresholdVectors[i])(s));
    }

    std::string weightsName = (boost::format("Layer%i_weights") % i).str();
    
    outputHistos.push_back(histoThreshLayer);

    TH2F* histoWeightsLayer=new TH2F(weightsName.c_str(),
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
    
    outputHistos.push_back(histoWeightsLayer);
    
  }
  

  return outputHistos;
  
}

TH1* NetworkToHistoTool::findHisto(std::string nameOfHisto,
				   std::vector<TH1*> & inputHistos) const
{
  std::vector<TH1*>::const_iterator inputBegin=inputHistos.begin();
  std::vector<TH1*>::const_iterator inputEnd=inputHistos.end();
  
  for ( std::vector<TH1*>::const_iterator 
	  inputIter=inputBegin;
	inputIter != inputEnd; 
	inputIter++) { 
    if ((*inputIter)->GetName() == nameOfHisto.c_str()) {
      return (*inputIter);
    }
  }
  throw std::runtime_error(" Could not find " + nameOfHisto + " histogram"); 
}



TFlavorNetwork* 
NetworkToHistoTool::fromHistoToTrainedNetwork(std::vector<TH1*> & inputHistos) const
{

  

  TH1* histoLayersInfo = findHisto("LayersInfo",inputHistos);

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

  // for (Int_t o=0;o<nHidden;++o)
  // {
  //   cout << " Hidden lay: " << o << " size: " << nHiddenLayerSize[o];
  // }

  Int_t nOutput=(Int_t)std::floor(histoLayersInfo->GetBinContent(2+nHidden)+0.5);
  // cout << " Output size: " << nOutput << endl;
  
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
   
    TH1* histoThreshLayer = findHisto(threName,inputHistos);

    for (Int_t s=0;s<layerSize;s++)
    {
      (*thresholdVector)(s) = histoThreshLayer->GetBinContent(s+1);
    }

    std::string weightsName = (boost::format("Layer%i_weights") % i).str();

    TH1* histoWeightsLayer = findHisto(weightsName, inputHistos);
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
  
  printf("WARNING: reading in histos with no normalization "
	 "you need to fix this\n"); 

  std::vector<TFlavorNetwork::Input> inputs; 
  for (int i = 0 ; i < nInput; i++) { 
    TFlavorNetwork::Input the_input; 
    the_input.name = boost::lexical_cast<std::string>(i); 
    the_input.offset = 0; 
    the_input.scale = 0; 
    inputs.push_back(the_input); 
  }
  TFlavorNetwork* trainedNetwork = 
    new TFlavorNetwork(inputs,
			nOutput,
			thresholdVectors,
			weightMatrices);
  return trainedNetwork;
  
}
