#include "UnitTests.hh"
#include <vector> 
#include <iostream>
#include <cstdlib>
#include "TTrainedNetwork.h"
#include "NNAdapters.hh"
#include "JetNet.hh"

int main(int narg, char* varg[]) { 
  test_trained(); 
  return 0; 
}

bool test_trained() { 
  std::vector<TTrainedNetwork::Input> inputs; 
  TTrainedNetwork::Input cat = {"cat",1,1}; 
  TTrainedNetwork::Input dog = {"dog",2,2}; 
  TTrainedNetwork::Input horse = {"horse",3,3}; 
  inputs.push_back(cat); 
  inputs.push_back(dog); 
  inputs.push_back(horse); 

  std::vector<TVectorD*> thresholds;
  std::vector<TMatrixD*> weights;

  for (int i = 0; i < 2; i++){ 
    thresholds.push_back(new TVectorD(3));
    weights.push_back(new TMatrixD(3,3));
  }

  std::cout << "before:\n"; 
  print_node_info(inputs); 

  TTrainedNetwork trained(inputs, 3, thresholds, weights); 

  std::vector<TTrainedNetwork::Input> read_back = trained.getInputs(); 

  std::cout << "after:\n"; 
  print_node_info(read_back); 

  // --- jn testing part
  int* nneurons = new int[4]; 
  nneurons[0] = 3; 
  nneurons[1] = 3; 
  nneurons[2] = 3; 
  nneurons[3] = 3; 
  JetNet* jn = new JetNet( 100, // testing events 
			   100, // training events
			   4, //layers
			   nneurons );
  jn->Init(); 

  std::vector<JetNet::InputNode> jn_input_info; 
  for (std::vector<TTrainedNetwork::Input>::const_iterator itr = 
	 read_back.begin(); 
       itr != read_back.end(); 
       itr++) { 
    jn_input_info.push_back(convert_node<JetNet::InputNode>(*itr));
  }
  jn->setInputNodes(jn_input_info); 
  
  std::vector<JetNet::InputNode> jn_read_back = jn->getInputNodes(); 

  std::cout << "after jn in / out:\n"; 
  print_node_info(jn_read_back); 


  TTrainedNetwork* jn_trained_out = getTrainedNetwork(*jn); 
  
  std::cout << "TTrainedNetwork from jn\n"; 
  print_node_info(jn_trained_out->getInputs()); 

  std::map<std::string,double> input_map; 
  std::vector<double> input_vector; 
  for (int i = 0; i < 3; i++){ 
    double value = i + 4; 
    double normed_value = 
      (value + inputs.at(i).offset ) * inputs.at(i).scale; 

    jn->SetInputs(i,normed_value); 
    input_map[inputs.at(i).name] = value; 
    input_vector.push_back(normed_value); 
  }

  jn->Evaluate(); 
  std::vector<double> ttrained_map_out = 
    jn_trained_out->calculateWithNormalization(input_map); 
  std::vector<double> ttrained_vector_out = 
    jn_trained_out->calculateOutputValues(input_vector); 
  
  for (int i = 0; i < 3; i++){ 
    std::cout << "output " << i << " -- JN: " << jn->GetOutput(i) 
	      << ", TTrainedNetwork map: " << ttrained_map_out.at(i) 
	      << ", TTrainedNetwork vec: " << ttrained_vector_out.at(i) 
	      << std::endl;
  }
  

  JetNet* new_jn = new JetNet( 100, // testing events 
			       100, // training events
			       4, //layers
			       nneurons );

  setTrainedNetwork(*new_jn,jn_trained_out); 
  std::cout << "new_jn from TTrainedNetwork\n"; 
  print_node_info(new_jn->getInputNodes()); 
  new_jn->Init(); 


  return true; 
}


// this is a work in progress

  // Int_t nInput=jn->GetInputDim();
  
  // cout << " create Trained Network object..." << endl;
  
  // TTrainedNetwork* trainedNetwork = jn->createTrainedNetwork();

  // cout << " now getting value with trained Network ";

  // double inputexample[9] = {norm_nVTX(1),
  // 			    norm_nTracksAtVtx(2),
  // 			    norm_nSingleTracks(0),
  // 			    norm_energyFraction(0.6),
  // 			    norm_mass(2500),
  // 			    norm_significance3d(4 ),
  // 			    norm_IP3D(3),
  // 			    norm_cat_pT(3),
  // 			    norm_cat_eta(1)};

  // for (Int_t i=0;i<nInput;++i){
  //   jn->SetInputs(i,inputexample[i]);
  // }

  
  // cronology.open(chronology_name.c_str(), ios_base::app);

  // jn->Evaluate();

  // cronology << "----------------CONSISTENCY CHECK-----------" << endl;
  // cout << "Result 0:" << jn->GetOutput(0);
  // cronology << "Result 0:" << jn->GetOutput(0);
  // cout << " Result 1:" << jn->GetOutput(1);
  // cronology << "Result 0:" << jn->GetOutput(1);
  // cout << " Result 2:" << jn->GetOutput(2) << endl;
  // cronology << " Result 2:" << jn->GetOutput(2) << endl;

  // cout << " Reading back old network " << endl;
  // jn->readBackTrainedNetwork(trainedNetwork);

  // cout <<" resetting input " << endl;
  // for (Int_t i=0;i<nInput;++i) {
  //   jn->SetInputs(i,inputexample[i]);
  // }

  // jn->Evaluate();
 
  // cout << "After reading back - Result 0:" << jn->GetOutput(0);
  // cronology << "After reading back - Result 0:" << jn->GetOutput(0);
  // // <<     " my: " << result[0] << endl;
  // cout << " After reading back - Result 1:" << jn->GetOutput(1);
  // cronology << "After reading back - Result 1:" << jn->GetOutput(1);
  // //<<     " my: " << result[1] << endl;
  // cout << " After reading back - Result 2:" << jn->GetOutput(2) << endl;
  // cronology << "After reading back - Result 2:" << jn->GetOutput(2);
  // // << " my: " << result[2] << endl;

  // cout << " Now getting histograms from trainingResult" << endl;
  // cronology << " Now getting histograms from trainingResult" << endl;

  // NetworkToHistoTool myHistoTool;

  // cout << " From network to histo..." << endl;
  // OwnerVector<TH1*> myHistos = myHistoTool.
  //   fromTrainedNetworkToHisto(trainedNetwork);

  // cout << " From histo to network back..." << endl;
  // TTrainedNetwork* trainedNetwork2 = myHistoTool.
  //   fromHistoToTrainedNetwork(myHistos);

  // cout << " reading back " << endl;
  // jn->readBackTrainedNetwork(trainedNetwork2);
   
  // cout <<" resetting input " << endl;
  // for (Int_t i=0;i<nInput;++i) {
  //   jn->SetInputs(i,inputexample[i]);
  // }

  // jn->Evaluate();

  // cout << "After reading back - Result 0:" << jn->GetOutput(0);
  // cronology << "After reading back - Result 0:" << jn->GetOutput(0);
  // // <<     " my: " << result[0] << endl;
  // cout << " After reading back - Result 1:" << jn->GetOutput(1);
  // cronology << "After reading back - Result 1:" << jn->GetOutput(1);
  // //<<     " my: " << result[1] << endl;
  // cout << " After reading back - Result 2:" << jn->GetOutput(2) << endl;
  // cronology << "After reading back - Result 2:" << jn->GetOutput(2);
  // // << " my: " << result[2] << endl;
  
  // cout << " Directly from the trainedNetwork read back from HISTOS...!" << endl;

  // std::vector<Double_t> inputData;
  // for (Int_t u=0;u<nInput;++u) {
  //   inputData.push_back(inputexample[u]);
  // }

  // std::vector<Double_t> outputData=trainedNetwork2->
  //   calculateOutputValues(inputData);

  // cout << "After reading back - Result 0:" << outputData[0] << endl;
  // cout << " After reading back - Result 1:" << outputData[1] << endl;
  // cout << " After reading back - Result 2:" << outputData[2] << endl;
   
  //   cout << " -------------------- " << endl;
  //   cout << " Writing OUTPUT histos " << endl;
  //   std::string histo_weights_name = out_dir + "/histoWeights.root"; 
  //   TFile* fileHistos=new TFile(histo_weights_name.c_str(),"recreate");
  //   NetworkToHistoTool histoTool;
  //   std::vector<TH1*> myHistos=histoTool.
  //     fromTrainedNetworkToHisto(trainedNetwork);

  //   for (std::vector<TH1*>::iterator histoIter = myHistos.begin();
  // 	 histoIter != myHistos.end(); 
  // 	 ++histoIter) {
  //     (*histoIter)->Write();
  //     delete *histoIter; 
  //     *histoIter = 0; 
  //   }

  //   // SUSPICIOUS: may be writing twice here too
  //   // fileHistos->Write();
  //   fileHistos->Close();
  //   delete fileHistos;
