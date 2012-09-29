#include "UnitTests.hh"
#include <vector> 
#include <iostream>
#include <stdexcept> 
#include <cstdlib>
#include <ctime> 
#include "TNeuralNetwork.h"
#include "TTrainedNetwork.h"
#include "NetworkToHistoTool.hh"
#include "NNAdapters.hh"
#include "JetNet.hh"
#include "TH1.h"
#include "TFile.h"

int main(int narg, char* varg[]) { 
  std::vector<int> hidden_layer_sizes; 
  for (int i = 1; i < narg; i++) { 
    printf("adding hidden layer %s\n", varg[i]); 
    hidden_layer_sizes.push_back(atoi(varg[i])); 
  }
  printf("running\n"); 
  test_trained(hidden_layer_sizes); 
  return 0; 
}

float get_rand(float range, float offset) { 
  float rand_base = float(rand()) / float(RAND_MAX); 
  rand_base += offset - 0.5; 
  rand_base *= range; 
  return rand_base; 
}

std::vector<double> test_histo_tool(const TNeuralNetwork* net, 
				    std::map<std::string, double> in, 
				    bool do_broken = false, 
				    std::string out_fname = "test_hists.root")
{
  NetworkToHistoTool histo_tool; 
  std::map<std::string,TH1*> hists = histo_tool.histsFromNetwork(net); 
  if (do_broken) { 
    hists.rbegin()->second->Fill(0.5,0.5); 
  }

  TFile* out_file = new TFile(out_fname.c_str(),"recreate"); 
  for (std::map<std::string,TH1*>::iterator itr = hists.begin(); 
       itr != hists.end(); 
       itr++){ 
    out_file->WriteTObject(itr->second); 
  }
  out_file->Close(); 

  TNeuralNetwork* from_hists = histo_tool.networkFromHists(hists); 
  return from_hists->calculateNormalized(in); 
}

std::vector<double> test_histo_tool(const TNeuralNetwork* net, 
				    std::vector<double> in, 
				    bool do_broken = false, 
				    std::string out_fname = "stripped.root")
{
  NetworkToHistoTool histo_tool; 
  std::map<std::string,TH1*> hists = histo_tool.histsFromNetwork(net); 
  if (do_broken) { 
    hists.rbegin()->second->Fill(0.5,0.5); 
  }

  hists.erase("InputsInfo"); 

  TFile* out_file = new TFile(out_fname.c_str(),"recreate"); 
  for (std::map<std::string,TH1*>::iterator itr = hists.begin(); 
       itr != hists.end(); 
       itr++){ 
    out_file->WriteTObject(itr->second); 
  }
  out_file->Close(); 

  TNeuralNetwork* from_hists = histo_tool.networkFromHists(hists); 
  return from_hists->calculateNormalized(in); 
}


bool test_trained(std::vector<int> layer_sizes) { 
  srand(time(0)); 
  std::vector<TNeuralNetwork::Input> inputs; 
  TNeuralNetwork::Input cat = {"cat",get_rand(),get_rand()}; 
  TNeuralNetwork::Input dog = {"dog",get_rand(),get_rand()}; 
  TNeuralNetwork::Input horse = {"horse",get_rand(),get_rand()}; 
  inputs.push_back(cat); 
  inputs.push_back(dog); 
  inputs.push_back(horse); 


  std::vector<TVectorD*> thresholds;
  std::vector<TMatrixD*> weights;

  int last_layer_size = inputs.size(); 
  for (int i = 0; i < layer_sizes.size() ; i++){ 
    int this_layer_size = layer_sizes.at(i); 
    thresholds.push_back(new TVectorD(this_layer_size));
    weights.push_back(new TMatrixD(last_layer_size,this_layer_size));
    last_layer_size = this_layer_size; 
  }
  thresholds.push_back(new TVectorD(3));
  weights.push_back(new TMatrixD(last_layer_size,3));
  

  std::cout << "before:\n"; 
  print_node_info(inputs); 

  TNeuralNetwork trained(inputs, 3, thresholds, weights); 

  std::vector<TNeuralNetwork::Input> read_back = trained.getInputs(); 

  std::cout << "after:\n"; 
  print_node_info(read_back); 

  // --- jn testing part
  int* nneurons = new int[layer_sizes.size() + 2]; 
  nneurons[0] = inputs.size(); 
  for (int i = 0; i < layer_sizes.size(); i++){ 
    printf("hidden %i: %i nodes\n",i,layer_sizes.at(i)); 
    nneurons[i + 1] = layer_sizes.at(i); 
  }
  nneurons[layer_sizes.size() + 1] = 3; 
  JetNet* jn = new JetNet( 100, // testing events 
			   100, // training events
			   layer_sizes.size() + 2, //layers
			   nneurons );
  jn->Init(); 

  std::vector<JetNet::InputNode> jn_input_info; 
  for (std::vector<TNeuralNetwork::Input>::const_iterator itr = 
	 read_back.begin(); 
       itr != read_back.end(); 
       itr++) { 
    jn_input_info.push_back(convert_node<JetNet::InputNode>(*itr));
  }
  jn->setInputNodes(jn_input_info); 
  
  std::vector<JetNet::InputNode> jn_read_back = jn->getInputNodes(); 

  std::cout << "after jn in / out:\n"; 
  print_node_info(jn_read_back); 


  TNeuralNetwork* jn_trained_out = getTrainedNetwork(*jn); 
  
  std::cout << "TNeuralNetwork from jn\n"; 
  print_node_info(jn_trained_out->getInputs()); 

  std::map<std::string,double> input_map; 
  std::vector<double> input_vector; 
  std::vector<double> raw_vector; 
  for (int i = 0; i < inputs.size(); i++){ 
    double value = float(rand()) / float(RAND_MAX) - 0.5; 
    double normed_value = 
      (value + inputs.at(i).offset ) * inputs.at(i).scale; 

    // normed_value += 1; 
    jn->SetInputs(i,normed_value); 
    input_map[inputs.at(i).name] = value; 
    input_vector.push_back(normed_value); 
    raw_vector.push_back(value); 
  }

  jn->Evaluate(); 
  
  std::cout << "calculate in a few ways: first with normalization..."; 
  
  std::vector<double> ttrained_map_out = 
    jn_trained_out->calculateNormalized(input_map); 
  std::cout << "now without...\n"; 
  std::vector<double> ttrained_vector_out; 
  try { 
    ttrained_vector_out = jn_trained_out->calculateOutputValues(input_vector); 
  }
  catch (std::out_of_range) { 
    std::cout << "out of range thrown in calculateOutputValues: size " << 
      "input_vector = " << input_vector.size() << " size input_map = " << 
      input_map.size() << std::endl; 
    throw; 
  }
  std::vector<double> ttrained_normvec_out = 
    jn_trained_out->calculateNormalized(raw_vector); 
  
  TTrainedNetwork* old_style_nn = convertNewToOld(jn_trained_out); 
  std::vector<double> old_style_out = old_style_nn->calculateOutputValues
    (input_vector); 
  TNeuralNetwork* new_style_nn = convertOldToNew(old_style_nn,inputs); 
  std::vector<double> new_style_out = 
    new_style_nn->calculateNormalized(input_map); 
  
  std::cout << "reading back...\n"; 
  int v_out_size = ttrained_vector_out.size();  
  int m_out_size = ttrained_map_out.size(); 
  if (v_out_size != m_out_size)
    printf("vector output size = %i, map = %i", v_out_size, m_out_size) ; 
  
  std::vector<double> histo_out = test_histo_tool
    (new_style_nn, input_map, false); 

  std::vector<double> stripped_hist_out = test_histo_tool
    (new_style_nn, input_vector, false); 

  for (int i = 0; i < 3; i++){ 
    std::cout << "output " << i << " -- JN: " << jn->GetOutput(i) 
	      << ", FlavNet map: " << ttrained_map_out.at(i) 
	      << ", FlavNet vec: " << ttrained_vector_out.at(i) 
	      << ", FlavNet normvec: " << ttrained_normvec_out.at(i)
	      << ", TrainedNet vec: " << old_style_out.at(i) 
	      << ", FlavNet from old: " << new_style_out.at(i) 
	      << ", From Histos: " << histo_out.at(i)
	      << ", From stripped hists: " << stripped_hist_out.at(i)
	      << std::endl;
  }
  

  JetNet* new_jn = new JetNet( 100, // testing events 
			       100, // training events
			       layer_sizes.size() + 2, //layers
			       nneurons );

  setTrainedNetwork(*new_jn,jn_trained_out); 
  std::cout << "new_jn from TNeuralNetwork\n"; 
  print_node_info(new_jn->getInputNodes()); 
  new_jn->Init(); 


  return true; 
}


// this is a work in progress

  // Int_t nInput=jn->GetInputDim();
  
  // cout << " create Trained Network object..." << endl;
  
  // TNeuralNetwork* trainedNetwork = jn->createTrainedNetwork();

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
  // TNeuralNetwork* trainedNetwork2 = myHistoTool.
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
