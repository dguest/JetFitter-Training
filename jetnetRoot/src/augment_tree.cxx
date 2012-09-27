#include "augment_tree.hh"
#include <string> 
#include <vector> 
#include <set> 
#include <cmath> // log
#include <utility> // pair
#include <cstdlib> // rand, srand
#include <iostream> // for debugging
#include "TNeuralNetwork.h"
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
// #include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "TFile.h"
#include "TTree.h"
#include "TROOT.h"

int augment_tree(std::string file_name, 
		 std::string nn_file, 
		 std::string tree_name, 
		 std::string output_file, 
		 std::vector<std::string> int_vec, 
		 std::vector<std::string> double_vec, 
		 std::set<std::string> subset, 
		 std::string extension, 
		 int max_entries, 
		 int start_entry, 
		 const unsigned options) 
{ 

  typedef std::vector<std::string> SVec;
  srand(0); 

  std::string out_tree_name = tree_name; 
  if (output_file.size() == 0) { 
    std::string output_file_ext = extension.size() ? extension : "_aug"; 
    output_file = file_name + output_file_ext; 
  }

  boost::shared_ptr<TNeuralNetwork> nn(get_nn(nn_file)); 
  typedef std::vector<TNeuralNetwork::Input> NNInputs; 
  NNInputs nn_inputs = nn->getInputs(); 
  std::set<std::string> input_name_set; 
  for (NNInputs::const_iterator itr = nn_inputs.begin(); 
       itr != nn_inputs.end(); itr++){ 
    input_name_set.insert(itr->name); 
  }

  TFile file(file_name.c_str()); 
  if (file.IsZombie() || !file.IsOpen() ) { 
    std::string err = " cannot be opened"; 
    throw std::runtime_error(file.GetName() + err); 
  }
  TTree* tree = dynamic_cast<TTree*> (file.Get(tree_name.c_str())); 
  if (!tree) { 
    std::string err = "%s in %s cannot be opened"; 
    std::string f = file.GetName(); 
    std::string t = tree->GetName(); 
    throw std::runtime_error( (boost::format(err) % t % f).str()); 
  }
  tree->SetBranchStatus("*",0); 
  
  TFile out_file(output_file.c_str(), "recreate"); 

  // hopefully this is owned by file
  boost::scoped_ptr<TTree> out_tree
    (new TTree(out_tree_name.c_str(), out_tree_name.c_str())); 


  boost::ptr_vector<double> double_buffer; 
  boost::ptr_vector<int> int_buffer; 
  typedef std::vector<std::pair<std::string, double*> > NNInputAddresses ; 
  NNInputAddresses nn_input_addresses; 
  typedef std::vector<std::pair<int* , double*> > NNConverters; 
  NNConverters nn_converters; 

  // set the branches
  for (SVec::const_iterator itr = int_vec.begin(); 
       itr != int_vec.end(); itr++) { 
    int* the_int = new int; 
    int_buffer.push_back(the_int); 
    tree->SetBranchStatus(itr->c_str(), 1); 
    if (tree->SetBranchAddress(itr->c_str(), the_int))
      throw std::runtime_error("could not find " + *itr); 

    if (subset.size() == 0 || subset.count(*itr)){
      out_tree->Branch(itr->c_str(), the_int); 
    }
    if (input_name_set.count(*itr) ) { 
      double* converted_int = new double; 
      double_buffer.push_back(converted_int); 
      nn_converters.push_back(std::make_pair(the_int, converted_int)); 
      nn_input_addresses.push_back(std::make_pair(*itr, converted_int)); 
    }
  }
  for (SVec::const_iterator itr = double_vec.begin(); 
       itr != double_vec.end(); itr++) { 
    double* the_double = new double; 
    double_buffer.push_back(the_double); 
    tree->SetBranchStatus(itr->c_str(), 1); 
    if (tree->SetBranchAddress(itr->c_str(), the_double))
      throw std::runtime_error("could not find " + *itr); 

    if (subset.size() == 0 || subset.count(*itr)) { 
      out_tree->Branch(itr->c_str(), the_double); 
    }
    if (input_name_set.count(*itr) ) { 
      nn_input_addresses.push_back(std::make_pair(*itr,the_double) ); 
    }
  }

  // the output nodes 
  double b_val, c_val, l_val; 
  out_tree->Branch(("Likelihood_b" + extension).c_str(), &b_val); 
  out_tree->Branch(("Likelihood_c" + extension).c_str(), &c_val); 
  out_tree->Branch(("Likelihood_l" + extension).c_str(), &l_val); 
  
  double log_cb, log_cu, log_bc, log_bu; 
  out_tree->Branch(("logCb" + extension).c_str(), &log_cb); 
  out_tree->Branch(("logCu" + extension).c_str(), &log_cu); 
  out_tree->Branch(("logBc" + extension).c_str(), &log_bc); 
  out_tree->Branch(("logBu" + extension).c_str(), &log_bu); 

  int n_entries = tree->GetEntries(); 
  if (max_entries < 0) max_entries = n_entries; 
  else max_entries = std::min(max_entries, n_entries); 

  int n_used_entries = max_entries - start_entry; 

  int one_percent = n_used_entries / 100; 

  for (int entry_n = start_entry; entry_n < max_entries; entry_n++) { 

    int n_processed = entry_n - start_entry; 
    if (options & augment::show_progress && 
	(n_processed % one_percent == 0 || entry_n + 1 == max_entries) ) {
      std::cout << boost::format
	("\r%.1fM of %.1fM entries processed (%.0f%%)") 
	% (float(n_processed) / 1e6) % (float(n_used_entries) / 1e6) 
	% (float(n_processed) * 100 / float(n_used_entries) ); 
      std::cout.flush(); 
    }
    tree->GetEntry(entry_n); 
    
    // do the int conversion 
    for (NNConverters::const_iterator itr = nn_converters.begin(); 
	 itr != nn_converters.end(); itr++) { 
      *itr->second = double(*itr->first); 
    }
    
    // build the nn input
    std::map<std::string, double> input; 
    for (NNInputAddresses::const_iterator itr = nn_input_addresses.begin(); 
	 itr != nn_input_addresses.end(); itr++) { 
      std::pair<std::string, double> input_pair = 
	std::make_pair(itr->first, *itr->second); 
      input.insert(input_pair); 
    }
    // run nn
    std::vector<double> nn_out = nn->calculateWithNormalization(input); 
    b_val = nn_out.at(0); 
    c_val = nn_out.at(1); 
    l_val = nn_out.at(2); 

    log_cb = log(c_val / b_val); 
    log_cu = log(c_val / l_val); 
    log_bc = log(b_val / c_val); 
    log_bu = log(b_val / l_val); 

    // fill tree 
    out_tree->Fill(); 
  }

  if (options & augment::show_progress ) { 
    std::cout << "\n"; 
  }

  
  // save tree 
  out_file.WriteTObject(out_tree.get()); 

  return 0; 

}

boost::shared_ptr<TNeuralNetwork> get_nn(std::string file_name) { 
  TFile file(file_name.c_str()); 
  if (file.IsZombie() || !file.IsOpen() ) { 
    std::string err = " cannot be opened"; 
    throw std::runtime_error(file.GetName() + err); 
  }

  TNeuralNetwork* the_network = dynamic_cast<TNeuralNetwork*>
    (file.Get("TNeuralNetwork")); 

  if (!the_network) 
    throw std::runtime_error("no TNeuralNetwork in " + file_name); 

  // take ownership
  gROOT->cd(); 
  boost::shared_ptr<TNeuralNetwork> export_net
    (dynamic_cast<TNeuralNetwork*>(the_network->Clone())); 
  return export_net; 
}
