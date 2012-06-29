#include <TTree.h>
#include <TFile.h>
#include <TH1F.h>
#include "TCollection.h"
#include "TKey.h"

#include <string.h>
#include <cmath>
#include <sstream>
#include <cstdlib> // rand
#include <ctime> // to seed srand
#include <boost/format.hpp>

#include "JetNet.hh"
// #include "NetworkToHistoTool.hh" // not currently used

#include "normedInput.hh"
#include "nnExceptions.hh"
#include <stdexcept>

#include "TFlavorNetwork.h"
#include "NNAdapters.hh"
#include "trainNN.hh"

#include <iostream>
#include <ostream> 
#include <fstream>
#include <streambuf>

#include "TMatrixD.h"
#include "TVectorD.h"




void trainNN(const TrainingInputs inputs, 
	     std::vector<int> n_hidden_layer_nodes, 
	     std::vector<InputVariableInfo> input_variables, 
	     const FlavorWeights flavor_weights, 
	     const unsigned bit_flags) {

  srand(time(0)); 

  // --- setup the output streams --- 

  std::string output_textfile_name = inputs.output_dir + "/run_info.txt"; 
  std::ofstream output_textfile; 
  std::streambuf* textfile_buffer; 
  bool do_file_output = 
    bit_flags & train::write_out_to_file ||
    !(bit_flags & train::verbose) ; 

  if (do_file_output) { 
    output_textfile.open(output_textfile_name.c_str()); 
    textfile_buffer = output_textfile.rdbuf(); 
  }
  else { 
    textfile_buffer = std::cout.rdbuf(); 
  }
  std::streambuf* normal_output; 
  if (bit_flags & train::write_out_to_file) { 
    normal_output = textfile_buffer; 
  }
  else { 
    normal_output = std::cout.rdbuf(); 
  }
  std::ostream textout(normal_output); 
  std::ostream verboseout(textfile_buffer); 

  if (bit_flags & train::verbose) { 
    verboseout << "using verbose output\n"; 
  }


  // --- setup the neural net ---

  textout << "--- starting trainNN ----\n"; 

  if (n_hidden_layer_nodes.size() == 0){
    if (bit_flags & train::throw_on_warn) 
      throw std::runtime_error("no hidden layer sizes given"); 
    
    textout << "WARNING: setting hidden layers to default sizes\n"; 
    n_hidden_layer_nodes.push_back(15); 
    n_hidden_layer_nodes.push_back(8); 
  }

  gROOT->ProcessLine("#include <TTree.h>"); 
  gROOT->ProcessLine("#include <TFile.h>"); 
  
  verboseout << "starting with settings: " << std::endl;
  verboseout << " nIterations: " << inputs.n_iterations << std::endl;
  for (int hidden_layer_n = 0; 
       hidden_layer_n < n_hidden_layer_nodes.size();  
       hidden_layer_n++) { 
    int nodes_here = n_hidden_layer_nodes.at(hidden_layer_n); 
    verboseout << " nodes layer " << hidden_layer_n + 1 << 
      ": " << nodes_here << std::endl;
  }
  verboseout << " nIterations: " << inputs.n_iterations << std::endl;
  
  // --- load training trees ---

  TFile* input_file = new TFile(inputs.file.c_str());
  TTree* in_tree = dynamic_cast<TTree*>(input_file->Get("SVTree"));
  if (! in_tree){ 
    throw std::runtime_error("Could not find SVTree in " + inputs.file); 
  }

  InputVariableContainer in_var; 
  if (input_variables.size() == 0) { 
    if (bit_flags & train::throw_on_warn) 
      throw std::runtime_error("no input variables given"); 

    textout << "WARNING: no input variables given, using defaults\n"; 
    in_var.set_hardcoded_defaults(in_tree); 
  }
  else { 
    for (std::vector<InputVariableInfo>::const_iterator 
	   itr = input_variables.begin(); 
	 itr != input_variables.end(); 
	 itr++){
      in_var.add_variable(*itr, in_tree); 
    }
  }

  if (bit_flags & train::verbose){ 
    textout << "input variables: \n"; 
    for (InputVariableContainer::const_iterator itr = in_var.begin(); 
	 itr != in_var.end(); itr++){ 
      textout << " " << *itr << std::endl;
    }
  }

  // training variables
  TeachingVariables teach; 
  
  if (in_tree->SetBranchAddress("weight",&teach.weight) ) { 
    if (bit_flags & train::throw_on_warn) 
      throw std::runtime_error("no 'weight' branch found"); 

    textout << "WARNING: no weight branch found, setting all weights to 1"
	      << std::endl; 
    teach.weight = 1; 
  }

  if (in_tree->SetBranchAddress("bottom",&teach.bottom) ||
      in_tree->SetBranchAddress("charm", &teach.charm) ||
      in_tree->SetBranchAddress("light", &teach.light) ) 
    throw std::runtime_error("SVTree in " + inputs.file + 
			     " missing one of: bottom, charm, light"); 

  int nlayer = n_hidden_layer_nodes.size() + 2; 
  int* nneurons = new int[nlayer];


  int numberinputs = in_var.size();
  nneurons[0] = numberinputs;

  for (int layer_n = 1; layer_n < nlayer - 1; layer_n++) { 
    nneurons[layer_n] = n_hidden_layer_nodes.at(layer_n - 1); 
  }
  
  nneurons[nlayer - 1] = 3; 

  //  float eventWeight(0);
  float trainingError(0);
  float testError(0);
  
  //setting learning parameters

  textout << " now providing training events " << std::endl;

  int n_entries = in_tree->GetEntries(); 
  textout << n_entries << " entries in chain\n"; 

  TrainingSettings settings; 
  settings.dilution_factor = DILUTION_FACTOR; 
  settings.n_training_events = 0; 
  settings.n_testing_events = 0; 

  if (inputs.n_training_events > 0) { 
    int target_entries = inputs.n_training_events; 
    if (target_entries > n_entries / settings.dilution_factor) 
      throw std::runtime_error
	( (boost::format("asked for %i entries, only have %i")
	   % (target_entries * settings.dilution_factor) 
	   % n_entries).str() ); 
    settings.n_training_events = target_entries; 
    settings.n_testing_events = target_entries; 
  }
  else { 
    for (Int_t i = 0; i < n_entries; i++) {

      if (i % 100000 == 0 ) {
	textout << " Counting training / testing events in sample."
	  " Looping over event " << i << std::endl;
      }
    
      if (i%settings.dilution_factor==0) settings.n_training_events+=1;
      if (i%settings.dilution_factor==1) settings.n_testing_events+=1;

    }
  }
  
  textout << " N. training events: " << settings.n_training_events << 
    " N. testing events: " << settings.n_testing_events << endl;

 
  JetNet* jn = new JetNet( settings.n_testing_events, 
			     settings.n_training_events, 
			     nlayer, 
			     nneurons );

  textout <<  " setting up JetNet... " << endl;
  jn->SetUpdatesPerEpoch( int(std::floor(float(settings.n_training_events)/
					 float(N_PATTERNS_PER_UPDATE) )));
  setup_jetnet(jn); 

  std::vector<JetNet::InputNode> jn_input_info; 
  for (std::vector<InputVariableInfo>::const_iterator itr = 
	 input_variables.begin(); 
       itr != input_variables.end(); 
       itr++) { 
    jn_input_info.push_back(convert_node<JetNet::InputNode>(*itr));
  }
  jn->setInputNodes(jn_input_info); 


  textout << " copying over training events " << std::endl;
  copy_training_events(verboseout, jn, in_var, teach, in_tree, 
		       settings, flavor_weights); 
  
  
  textout << " copying over testing events " << std::endl;
  copy_testing_events(verboseout, jn, in_var, teach, in_tree, 
		      settings, flavor_weights); 

  //normalize inputvariables?
  //jn->Normalize();

  jn->Shuffle(true,false);

  // -- the code below the assert is almost certainly broken
  assert(inputs.restart_training_from == 0); 
  jn->Init();
  // if (inputs.restart_training_from == 0) {
  //   jn->Init();
  // }
  // else {
  //   std::stringstream weight_name; 
  //   weight_name << inputs.output_dir; 
  //   weight_name << "/Weights" << inputs.restart_training_from << ".root"; 
  //   jn->ReadFromFile(weight_name.str().c_str());
  // }
  
  float minimumError = 1e10;
  int epochesWithRisingError = 0;
  int epochWithMinimum = 0;

  int updatesPerEpoch = jn->GetUpdatesPerEpoch();

  //prepare output stream
  
  std::string chronology_name = inputs.output_dir + "/trainingCronology.txt"; 

  ofstream cronology(chronology_name.c_str(),ios_base::out);
  if (! cronology ) 
    throw std::runtime_error("Could not write " + chronology_name); 
  
  cronology << jn; 

  cronology << "--------------HISTORY-----------------" << endl;
  cronology << "History of iterations: " << endl;
  cronology.close();

  //prepare training histo
  TH1F* histoTraining = new TH1F
    ("training","training",
     (int)std::floor((float)inputs.n_iterations/10.+0.5),
     1,std::floor((float)inputs.n_iterations/10.+1.5));

  TH1F* histoTesting=new TH1F
    ("testing","testing",
     (int)std::floor((float)inputs.n_iterations/10.+0.5),
     1,std::floor((float)inputs.n_iterations/10.+1.5));


  for(int epoch=inputs.restart_training_from+1;epoch<=inputs.n_iterations;++epoch){
    if (epoch!=inputs.restart_training_from+1) {
      trainingError = jn->Train();
    }

    if (epoch%10==0 || epoch==inputs.restart_training_from+1) {

      testError = jn->Test(); 

      histoTraining->Fill(epoch/10.0,trainingError);
      histoTesting->Fill(epoch/10.0,testError);

      if (testError<minimumError) {
	minimumError=testError;
	epochesWithRisingError=0;
	epochWithMinimum=epoch;
      }
      else {
	epochesWithRisingError+=10;
	// NOTE: changed Sat May 12 17:04:56 CEST 2012, 
	//       should ask giacinto why he was doing this in the first place
	// As of Thu Jun 28 20:30:08 CEST 2012 this is turned on with the 'g'
	// flag
	if (trainingError>testError && 
	    (bit_flags & train::push_min_to_xtest)) {
	  epochWithMinimum=epoch;
	}
      }
      
      cronology.open(chronology_name.c_str(),ios_base::app);
      if (epochesWithRisingError>300) {
	// Sat May 12 17:06:06 CEST 2012 --- commented this out 
	if (trainingError < minimumError || 
	    !(bit_flags & train::req_training_lt_min) ) { 
	  textout << " End of training. Minimum already on epoch: " 
		    << epochWithMinimum << endl;
	  cronology << " End of training. Minimum already on epoch: " 
		    << epochWithMinimum << endl;
	break;
	} 
      }
      
      cronology << "Epoch: [" << epoch <<
	"] Error: " << trainingError << 
	" Test: " << testError << endl;

      textout << "Epoch: [" << epoch <<
	"] Error: " << trainingError << 
	" Test: " << testError << endl;

      cronology.close();
      
      std::string weight_name = inputs.output_dir + "/Weights"; 
      TString name(weight_name.c_str());
      name+=epoch;
      name+=".root";

      TFile* file = new TFile(name,"recreate");
      TFlavorNetwork* trainedNetwork = getTrainedNetwork(*jn);
      file->WriteTObject(trainedNetwork); 


      
      
      // trainedNetwork->Write();
      // file->Write(); //*** SUSPICIOUS: may result in two copies in the file
      
      // --- write in variable tree too 
      // in_var.write_to_file(file); 

      file->Close();
      delete file;

    }
  }
      
  jn->writeNetworkInfo(1);
  jn->writeNetworkInfo(2);
  //  jn->writeNetworkInfo(3);
  //  jn->writeNetworkInfo(4);
  //  jn->writeNetworkInfo(5);

  // ==================================================================
  // ======== end of training (good place to chop in half) ============
  // ==================================================================

  if (epochWithMinimum!=0) {
    cronology << "Minimum stored from Epoch: " << epochWithMinimum << endl;
  }  
  else {
    cronology << "Minimum not reached" << endl;
  }

  cronology.close();

  if (epochWithMinimum != 0) {

    std::string weights_out_name = inputs.output_dir + "/Weights"; 
    TString min_file_name(weights_out_name.c_str());
    min_file_name += epochWithMinimum;
    min_file_name += ".root";


    TFile min_file(min_file_name);
    TFlavorNetwork* trainedNetwork = 
      dynamic_cast<TFlavorNetwork*>(min_file.Get("TFlavorNetwork"));
    
    // textout << " Reading back network with minimum" << endl;
    // setTrainedNetwork(*jn,trainedNetwork);

    std::string min_weights_name = inputs.output_dir + "/weightMinimum.root"; 
    TFile out_min(min_weights_name.c_str(),"recreate");
    out_min.WriteTObject(trainedNetwork);

    copy_cat_trees(out_min, *input_file); 

  } 
  else {
    textout << " using network at last iteration (minimum not reached)" 
	      << endl;
  }
  
  std::string training_info_name = inputs.output_dir + "/trainingInfo.root"; 
  TFile* histoFile=new TFile(training_info_name.c_str(),"recreate");
  histoTraining->Write();
  histoTesting->Write();
  // histoFile->Write();
  histoFile->Close();
  delete histoFile;

  delete nneurons; 

}

// TODO: it would be better to replace these category trees with some other
// data format
void copy_cat_trees(TFile& dest_file, const TFile& source_file) { 
  const TList* keys = source_file.GetListOfKeys(); 
  TIter the_iter(keys); 
  for (TIter obj = the_iter.Begin(); obj != the_iter.End(); ++obj) { 
    TKey* key = dynamic_cast<TKey*>(*obj); 
    assert(key); 
    std::string name = key->GetName(); 
    std::string obj_name = key->GetClassName(); 
    if (obj_name == "TTree" && name.find("_cat") != std::string::npos) { 
      TTree* the_tree = static_cast<TTree*>(key->ReadObj()); 
      assert(the_tree); 
      dest_file.WriteTObject(the_tree); 
    }
  }
}

float get_entry_weight(const TeachingVariables& teach, 
		       const FlavorWeights& w) { 
  float event_weight = -1; 
  if (teach.bottom) event_weight = teach.weight * w.bottom; 
  else if (teach.charm) event_weight = teach.weight * w.charm; 
  else if (teach.light) event_weight = teach.weight * w.light; 
  return event_weight; 
}

float adjusted_keep_prob(float old_prob, const FlavorWeights& w) { 
  int n_skipped = (w.bottom < 0) + (w.light < 0) + (w.charm < 0); 
  assert(n_skipped < 3); 
  return old_prob / (1 - n_skipped / 3.0); 
}


int copy_testing_events(std::ostream& stream, 
			JetNet* jn, 
			const InputVariableContainer& in_buffer, 
			TeachingVariables& teach, 
			TTree* in_tree,
			const TrainingSettings& s, 
			const FlavorWeights& w) { 

  int testing_counter=0;
  int n_entries = in_tree->GetEntries(); 
  for (Int_t i = 0; i < n_entries; i++) {
    
    if (i % 100000 == 0 ) {
      stream << " Copying over testing events."
	" Looping over event " << i << std::endl;
    }
    
    if (i % s.dilution_factor !=1 ) continue;

    // if (inputs.n_training_events > 0) { 

    float number_events_remaining = 
      float(n_entries - i) / float(s.dilution_factor); 
    float number_events_needed = 
      s.n_testing_events - testing_counter; 
    float keep_event_probibility = 
      number_events_needed / number_events_remaining; 

    keep_event_probibility = adjusted_keep_prob(keep_event_probibility, w); 

    float f_rand = float(rand()) / float(RAND_MAX); 
    if (f_rand > keep_event_probibility) continue; 
    // }
    
    in_tree->GetEntry(i);

    float event_weight = get_entry_weight(teach, w); 
    
    if (event_weight < 0) continue; 

    for (int var_num = 0; var_num < in_buffer.size(); var_num++){ 
      jn->SetInputTestSet( testing_counter, 
			   var_num, 
			   in_buffer.at(var_num).get_normed() );
    }

    jn->SetOutputTestSet( testing_counter, 0, teach.bottom );
    jn->SetOutputTestSet( testing_counter, 1, teach.charm );
    jn->SetOutputTestSet( testing_counter, 2, teach.light );

    jn->SetEventWeightTestSet( testing_counter, event_weight); 

    assert(testing_counter <= s.n_testing_events); 

    testing_counter+=1;

    //not used!
    //    eventWeight=weight;
  }
    
  if (testing_counter != s.n_testing_events){
    std::string form_str = 
      "counter up to: %i while events in testing sample are %i"; 
    std::string err = (boost::format(form_str) % 
		       testing_counter % 
		       s.n_testing_events).str(); 
    throw std::runtime_error(err); 
  }
  return testing_counter; 

}

int copy_training_events(std::ostream& stream, JetNet* jn, 
			 const InputVariableContainer& in_buffer, 
			 TeachingVariables& teach, 
			 TTree* in_tree,
			 const TrainingSettings& s, 
			 const FlavorWeights& w) { 

  int n_entries = in_tree->GetEntries(); 
  int training_counter=0;
  for (Int_t i = 0; i < n_entries; i++) {
    
    if (i % 100000 == 0 ) {
      stream << " Copying over training events. Looping over event " 
	     << i << std::endl;
    }

    if (i % s.dilution_factor != 0) continue;

    float number_events_remaining = 
      float(n_entries - i) / float(s.dilution_factor); 
    float number_events_needed = 
      s.n_training_events - training_counter; 
    float keep_event_probibility = 
      number_events_needed / number_events_remaining; 

    keep_event_probibility = adjusted_keep_prob(keep_event_probibility, w); 

    float f_rand = float(rand()) / float(RAND_MAX); 
    if (f_rand > keep_event_probibility) continue; 


    in_tree->GetEntry(i);

    float event_weight = get_entry_weight(teach, w); 

    if (event_weight < 0) continue;

    for (int var_num = 0; var_num < in_buffer.size(); var_num++){ 
      jn->SetInputTrainSet( training_counter, 
			    var_num, 
			    in_buffer.at(var_num).get_normed() );
    }

    jn->SetOutputTrainSet( training_counter, 0, teach.bottom );
    jn->SetOutputTrainSet( training_counter, 1, teach.charm );
    jn->SetOutputTrainSet( training_counter, 2, teach.light );

    jn->SetEventWeightTrainSet(training_counter, event_weight );

    assert(training_counter <= s.n_training_events); 

    training_counter+=1;

  }

  if (training_counter != s.n_training_events){

    std::string form_str = 
      "counter up to: %i while events in training sample are %i"; 
    std::string err = (boost::format(form_str) % 
		       training_counter % 
		       s.n_training_events).str(); 
    throw std::runtime_error(err); 
  }
  return training_counter; 

}

void setup_jetnet(JetNet* jn) { 
  //  jn->SetMSTJN(4,12); Fletscher-Rieves (Scaled Conj Grad)

  
  jn->SetPatternsPerUpdate( N_PATTERNS_PER_UPDATE );
  jn->SetUpdatingProcedure( 0 );
  jn->SetErrorMeasure( 0 );
  jn->SetActivationFunction( 1 );
  //  jn->SetLearningRate( 0.5);//0.8 || _2 =0.5 _3=0.05 _4=0.15
  jn->SetLearningRate( 0.5);//0.8 //move to 20 for _3 _4 = 0.15
  //  jn->SetMomentum( 0.3 );//0.3 //is now 0.5 || _2 = 0.3 _3 = 0.03 _4 = 0.05
  jn->SetMomentum( 0.03 );//0.3 //is now 0.5
  jn->SetInitialWeightsWidth( 1. );
  //  jn->SetLearningRateDecrease( 0.992 );
  //  jn->SetLearningRateDecrease( 0.99 );//0.992 || _2 = 0.99 _3 = 0.98 _4=0.99
  jn->SetLearningRateDecrease( 0.99 );//0.992
}

bool is_flavor_tagged(const TeachingVariables& t){ 
  bool has_tag = t.bottom || t.charm || t.light; 
  if (has_tag &&
      t.bottom + t.charm + t.light != 1) 
    throw std::runtime_error("jet was tagged with int != 1"); 
  return has_tag; 
}

void dump_nn_settings(ostream& cronology, const JetNet* jn) {   
  cronology << "-------------SETTINGS----------------" << endl;
  cronology << "Epochs: " << jn->GetEpochs() << std::endl;
  cronology << "Updates Per Epoch: " << jn->GetUpdatesPerEpoch() << std::endl;
  cronology << "Updating Procedure: " << jn->GetUpdatingProcedure() 
	    << std::endl;
  cronology << "Error Measure: " << jn->GetErrorMeasure() << std::endl;
  cronology << "Patterns Per Update: " << jn->GetPatternsPerUpdate() 
	    << std::endl;
  cronology << "Learning Rate: " << jn->GetLearningRate() << std::endl;
  cronology << "Momentum: " << jn->GetMomentum() << std::endl;
  cronology << "Initial Weights Width: " << jn->GetInitialWeightsWidth() 
	    << std::endl;
  cronology << "Learning Rate Decrease: " << jn->GetLearningRateDecrease() 
	    << std::endl;
  cronology << "Activation Function: " << jn->GetActivationFunction() 
	    << std::endl;
  cronology << "-------------LAYOUT------------------" << endl;
  cronology << "Input variables: " << jn->GetInputDim() << endl;
  cronology << "Output variables: " << jn->GetOutputDim() << endl;
  cronology << "Hidden layers: " << jn->GetHiddenLayerDim() << endl;
  cronology << "Layout : ";
  for (Int_t s=0; s < jn->GetHiddenLayerDim() + 2; ++s) {
    cronology << jn->GetHiddenLayerSize(s);
    if (s < jn->GetHiddenLayerDim()+1) cronology << "-";
  }
  cronology << endl;
}

std::ostream& operator<<(std::ostream& in, const JetNet* jn) { 
  dump_nn_settings(in, jn); 
  return in; 
}
