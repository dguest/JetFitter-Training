// python wrapper for JetNet based training algorithm
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <set> 
#include <iostream>
#include <stdexcept>
#include "trainNN.hh"
#include "testNN.hh"
#include "augment_tree.hh"
#include "nnExceptions.hh"
#include "pyparse.hh"
#include "makeTestNtuple.hh"
#include "pynn.hh"


static const char* train_doc_string = 
  "run the neural net. \n"
  "Keywords:\n"
  "reduced_dataset\n"
  "output_directory\n"
  "n_iterations\n"
  "dilution_factor\n"
  "normalization\n"
  "nodes\n"
  "restart_training_from\n"
  "debug"; 

extern "C" PyObject* train_py(PyObject *self, 
			      PyObject *args, 
			      PyObject *keywds)
{
  TrainingInputs inputs; 

  const char* input_file; 
  const char* output_dir = "weights"; 
  inputs.n_iterations = 10; 
  PyObject* normalization = 0; 
  PyObject* nodes = 0; 
  inputs.restart_training_from = 0; 
  PyObject* flavor_weights = 0; 
  inputs.n_training_events = -1; 
  bool debug = false; 

  const char *kwlist[] = {
    "reduced_dataset",
    "output_directory", 
    "n_iterations", 
    "normalization", 
    "nodes", 
    "restart_training_from",
    "flavor_weights", 
    "n_training_events_target", 
    "debug", 
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords
      (args, keywds, "s|siOOiOib", 
       // this function should take a const, and 
       // may be changed, until then we'll cast
       const_cast<char**>(kwlist),
       &input_file, 
       &output_dir, 
       &inputs.n_iterations, 
       &normalization, 
       &nodes, 
       &inputs.restart_training_from, 
       &flavor_weights, 
       &inputs.n_training_events, 
       &debug)
      ) return NULL;

  inputs.file = input_file; 
  inputs.output_dir = output_dir; 

  // --- parse input variables
  std::vector<InputVariableInfo> input_variable_info; 

  input_variable_info = parse_input_variable_info(normalization); 
  if (PyErr_Occurred()) return 0; 


  // --- parse node configuration 
  std::vector<int> node_vec = parse_int_seq(nodes); 
  if (PyErr_Occurred()) return 0; 

  // --- parse flavor weights 
  typedef std::map<std::string, double>::const_iterator SDMapItr; 
  std::map<std::string, double> flavor_weights_map; 
  flavor_weights_map["bottom"] = 1; 
  flavor_weights_map["charm"] = 1; 
  flavor_weights_map["light"] = 5; 

  std::map<std::string, double> new_flavor_weights 
    = parse_double_dict(flavor_weights); 
  if (PyErr_Occurred()){
    PyErr_SetString(PyExc_TypeError,
		    "flavor_weights should be a dict of the form: "
		    "{'bottom':x, 'charm':y, 'light':z}"); 
    return 0;
  }
    
  for ( SDMapItr itr = new_flavor_weights.begin(); 
	itr != new_flavor_weights.end(); itr++){ 
    flavor_weights_map[itr->first] = itr->second; 
  }

  if (flavor_weights_map.size() != 3) { 
    flavor_weights_map.erase("bottom"); 
    flavor_weights_map.erase("charm"); 
    flavor_weights_map.erase("light"); 
    std::string badkey = "I don't know what "; 
    for (SDMapItr itr = flavor_weights_map.begin(); 
	 itr != flavor_weights_map.end(); itr++){ 
      badkey.append(itr->first); 
      badkey.append(" "); 
    }
    badkey.append("is, allowed flavors are 'bottom', 'charm', 'light'"); 
	   
    PyErr_SetString(PyExc_LookupError, badkey.c_str()); 
    return 0; 
  }
  FlavorWeights flavor_weights_struct; 
  flavor_weights_struct.bottom = flavor_weights_map["bottom"]; 
  flavor_weights_struct.charm = flavor_weights_map["charm"]; 
  flavor_weights_struct.light = flavor_weights_map["light"]; 

  // --- dump debug info 
  if (debug){ 
    printf("in = %s, out dir = %s, itr = %i, rest from = %i, nodes: (", 
  	   inputs.file.c_str(), inputs.output_dir.c_str(), 
	   inputs.n_iterations, inputs.restart_training_from); 
    for (std::vector<int>::const_iterator itr = node_vec.begin(); 
	 itr != node_vec.end(); 
	 itr++){
      if (itr != node_vec.begin()) printf(","); 
      printf("%i", *itr); 
    }
    printf(")\n"); 
    
    for (std::vector<InputVariableInfo>::const_iterator itr = 
	   input_variable_info.begin(); 
	 itr != input_variable_info.end(); 
	 itr++){ 
      printf("name = %s, offset = %f, scale = %f\n", 
	     itr->name.c_str(), itr->offset, itr->scale); 
    }
    
  }
  else { 


    try { 
      trainNN(inputs, node_vec, input_variable_info, flavor_weights_struct); 
    }
    catch (const LoadReducedDSException& e){ 
      PyErr_SetString(PyExc_IOError,"could not load dataset"); 
      return NULL; 
    }
    catch (const NNException& e) { 
      PyErr_SetString(PyExc_StandardError,"generic nn exception"); 
      return NULL; 
    }
    catch (const std::runtime_error& e) { 
      PyErr_SetString(PyExc_StandardError,e.what()); 
      return NULL; 
    }
  }  

  Py_INCREF(Py_None);

  return Py_None;
}

static const char* test_doc_string = 
  "test the neural net. \n"
  "Keywords:\n"
  "reduced_dataset\n"
  "weights_file\n"
  "dilution_factor\n"
  "use_sd\n"
  "with_ip3d\n"; 

extern "C" PyObject* test_py(PyObject *self, 
			     PyObject *args, 
			     PyObject *keywds)
{
  const char* input_file; 
  const char* training_file = "weights/weightMinimum.root"; 
  int dilution_factor = 2; 
  int nodes_first_layer = 10; 
  int nodes_second_layer = 9; 
  bool use_sd = false; 
  bool with_ip3d = true; 
  bool debug = false; 
  const char* out_file = "all_hists.root"; 

  const char *kwlist[] = {
    "reduced_dataset",
    "weights_file", 
    "dilution_factor",
    "use_sd",
    "with_ip3d",
    "output_file", 
    "debug", 
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|sibbsb", 
				   // this function should take a const, and 
				   // may be changed, until then we'll cast
				   const_cast<char**>(kwlist),
				   &input_file,  
				   &training_file,
				   &dilution_factor, 
				   &use_sd, 
				   &with_ip3d, 
				   &out_file, 
				   &debug))
    return NULL;

  if (debug){ 
    printf("in ds = %s, train = %s, dil = %i\n", 
  	   input_file, training_file, dilution_factor); 
    
    
  }
  else { 
    try { 
      testNN(input_file, 
	     training_file,
	     dilution_factor,
	     // nodes_first_layer, 
	     // nodes_second_layer, 
	     use_sd,
	     with_ip3d, 
	     out_file); 
    }
    catch (const MissingLeafException& e) { 
      std::string error = "could not find leaf " + e.leaf_name() + 
	" in chain" + e.chain_name() + " file " + input_file; 
      PyErr_SetString(PyExc_IOError, error.c_str()); 
      return NULL; 
    }
    catch (const NNException& e) { 
      PyErr_SetString(PyExc_StandardError,"generic nn exception in testNN"); 
      return NULL; 
    }
    catch (const std::runtime_error& e) { 
      PyErr_SetString(PyExc_StandardError,e.what()); 
      return NULL; 
    }
  }

  Py_INCREF(Py_None);

  return Py_None;
}

PyObject* py_augment_tree(PyObject *self, 
			  PyObject *args, 
			  PyObject *keywds)
{
  const char* file_name; 
  const char* nn_file; 
  const char* tree_name = "SVTree"; 
  const char* output_file = ""; 
  PyObject* int_leaves = 0; 
  PyObject* double_leaves = 0; 
  PyObject* py_subset = 0; 
  const char* extension = "Aug"; 
  int max_entries = -1; 
  bool show_progress = false; 
  const char* kwlist[] = {
    "in_file",
    "nn_file", 
    "tree", 
    "out_file", 
    "ints", 
    "doubles", 
    "subset", 
    "extension", 
    "max_entries",
    "show_progress", 
    NULL};
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     "ss|ssOOOsib", 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(kwlist),
     &file_name,
     &nn_file, 
     &tree_name, 
     &output_file, 
     &int_leaves, 
     &double_leaves, 
     &py_subset, 
     &extension, 
     &max_entries, 
     &show_progress); 

  if (!ok) return NULL;

  std::vector<std::string> int_vec = parse_string_list(int_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<std::string> double_vec = parse_string_list(double_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<std::string> subset_vec = parse_string_list(py_subset); 
  if (PyErr_Occurred()) return NULL; 

  std::set<std::string> subset(subset_vec.begin(), subset_vec.end()); 

  unsigned options = 0; 
  if (show_progress) { 
    options |= augment::show_progress; 
  }

  int ret_code = 0; 
  try { 
    ret_code = augment_tree
      (file_name, 
       nn_file, 
       tree_name, 
       output_file, 
       int_vec, 
       double_vec, 
       subset, 
       extension, 
       max_entries, 
       options); 
  }
  catch (const std::runtime_error& e) { 
    PyErr_SetString(PyExc_IOError,e.what()); 
    return 0; 
  }
  
  return Py_BuildValue("i",ret_code); 

  // --- make sure you call INCREF if you return Py_None
  // Py_INCREF(Py_None);
  // return Py_None;
}


extern "C" PyObject* make_test_ntuple(PyObject *self, 
				      PyObject *args, 
				      PyObject *keywds)
{
  const char* input_weights_name; 
  const char* input_dataset_name; 
  const char* output_file_name; 
  const char* output_tree_name = "performance"; 
  bool debug = false; 

  const char *kwlist[] = {
    "weights_file",
    "reduced_dataset", 
    "output_file", 
    "output_tree", 
    "debug", 
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords
      (args, keywds, "sss|sb", 
       // this function should take a const, and 
       // may be changed, until then we'll cast
       const_cast<char**>(kwlist),
       &input_weights_name,
       &input_dataset_name, 
       &output_file_name, 
       &output_tree_name, 
       &debug)
      )
    return NULL;


  if (debug){ 
    printf("in wt = %s, in ds = %s, out file = %s, out tree = %s\n", 
  	   input_weights_name, input_dataset_name, 
	   output_file_name, output_tree_name); 
  }

  else{ 
    IONames io_names = { 
      input_weights_name, 
      input_dataset_name, 
      output_file_name, 
      output_tree_name
    }; 

    try { 
      makeTestNtuple(io_names); 
    }
    catch (const LoadReducedDSException& e) { 
      std::string error = "could not load --- " + e.info(); 
      PyErr_SetString(PyExc_IOError,error.c_str()); 
      return NULL; 
    }
    catch (const NNException& e) { 
      PyErr_SetString(PyExc_StandardError,"generic nn exception"); 
      return NULL; 
    }
    catch (const std::runtime_error& e) { 
      PyErr_SetString(PyExc_StandardError,e.what()); 
      return NULL; 
    }

  }

  Py_INCREF(Py_None);

  return Py_None;
}

static const char* ntuple_doc_string = 
  "input_weights\n"
  "input_dataset\n"
  "output_file\n"
  "output_tree\n"
  "debug";


static PyMethodDef keywdarg_methods[] = {
  // The cast of the function is necessary since PyCFunction values
  // only take two PyObject* parameters, and keywdarg() takes
  // three.
  {"trainNN", (PyCFunction)train_py, 
   METH_VARARGS | METH_KEYWORDS,
   train_doc_string},
  {"testNN", (PyCFunction)test_py, 
   METH_VARARGS | METH_KEYWORDS,
   test_doc_string},
  {"makeNtuple", (PyCFunction)make_test_ntuple, 
   METH_VARARGS | METH_KEYWORDS,
   ntuple_doc_string},
  {"augment_tree", (PyCFunction)py_augment_tree, 
   METH_VARARGS | METH_KEYWORDS,
   "augments a tree (duh)"},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" PyMODINIT_FUNC initpynn(void)
{
  Py_InitModule("pynn", keywdarg_methods);
}


//=======================================================
//========= helpers ========================
//=======================================================

std::vector<InputVariableInfo> parse_input_variable_info(PyObject* in_dict)
{
  std::vector<InputVariableInfo> input_variable_info; 
  if (in_dict == 0) { 
    return input_variable_info; 
  }

  bool ok = PyDict_Check(in_dict); 
  if (!ok) { 
    throw ParseException(); 
  }

  PyObject* key = 0; 
  PyObject* value = 0; 
  Py_ssize_t pos = 0; 
  while (PyDict_Next(in_dict, &pos, &key, &value) ) { 
    bool ok_key = PyString_Check(key); 
    bool ok_tuple = PyTuple_Check(value); 
    if (!ok_key || !ok_tuple) { 
      throw ParseException(); 
    }
    InputVariableInfo this_input; 
    this_input.name = PyString_AsString(key); 
    
    int tup_size = PyTuple_Size(value); 
    if (tup_size != 2) { 
      throw ParseException(); 
    }
    PyObject* offset = PyTuple_GetItem(value,0); 
    PyObject* scale = PyTuple_GetItem(value,1); 
    bool ok_offset = PyFloat_Check(offset); 
    bool ok_scale = PyFloat_Check(scale); 
    if (!ok_offset || !ok_scale){ 
      throw ParseException(); 
    }
    this_input.offset = PyFloat_AsDouble(offset); 
    this_input.scale = PyFloat_AsDouble(scale); 
    
    input_variable_info.push_back(this_input); 
  }
  
  return input_variable_info; 
}
