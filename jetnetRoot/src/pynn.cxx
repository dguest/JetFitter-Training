// python wrapper for JetNet based training algorithm
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <iostream>
#include "trainNN.hh"
#include "testNN.hh"
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
  const char* input_file; 
  const char* output_dir = "weights"; 
  int n_iterations = 10; 
  int dilution_factor = 2; 
  PyObject* normalization = 0; 
  PyObject* nodes = 0; 
  int restart_training_from = 0; 
  PyObject* flavor_weights = 0; 
  int n_training_events_target = -1; 
  bool debug = false; 

  const char *kwlist[] = {
    "reduced_dataset",
    "output_directory", 
    "n_iterations", 
    "dilution_factor",
    "normalization", 
    "nodes", 
    "restart_training_from",
    "flavor_weights", 
    "n_training_events_target", 
    "debug", 
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords
      (args, keywds, "s|siiOOiOib", 
       // this function should take a const, and 
       // may be changed, until then we'll cast
       const_cast<char**>(kwlist),
       &input_file, 
       &output_dir, 
       &n_iterations, 
       &dilution_factor, 
       &normalization, 
       &nodes, 
       &restart_training_from, 
       &flavor_weights, 
       &n_training_events_target, 
       &debug)
      ) return NULL;

  // --- parse input variables
  std::vector<InputVariableInfo> input_variable_info; 
  try {
    input_variable_info = parse_input_variable_info(normalization); 
  }
  catch (const ParseException& e) { 
    PyErr_SetString(PyExc_TypeError, 
		    "expected a dict, "
		    "key = varname, value = (offset, scale)"); 
    return 0; 
  }

  // --- parse node configuration 
  std::vector<int> node_vec; 
  try {
    node_vec = parse_int_tuple(nodes); 
  }
  catch(const ParseException& e) { 
    PyErr_SetString(PyExc_TypeError,
		    "expected a tuple of int, found something else"); 
    return 0;
  }

  // --- parse flavor weights 
  typedef std::map<std::string, double>::const_iterator SDMapItr; 
  std::map<std::string, double> flavor_weights_map; 
  flavor_weights_map["bottom"] = 1; 
  flavor_weights_map["charm"] = 1; 
  flavor_weights_map["light"] = 5; 
  try {
    std::map<std::string, double> new_flavor_weights;
    new_flavor_weights = parse_double_dict(flavor_weights); 
    
    for ( SDMapItr itr = new_flavor_weights.begin(); 
	  itr != new_flavor_weights.end(); itr++){ 
      flavor_weights_map[itr->first] = itr->second; 
    }
  }
  catch (const ParseException& e){ 
    PyErr_SetString
      (PyExc_TypeError,
       "flavor_weights should be a dict of the form: "
       "{'bottom':x, 'charm':y, 'light':z}"); 
    return 0;
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
  	   input_file, output_dir, n_iterations, restart_training_from); 
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

    if (node_vec.size() != 2){ 
      PyErr_SetString(PyExc_ValueError,
		      "node tuple must have two entries (for now)"); 
      return 0;
    }

    try { 
      trainNN(input_file, 
	      output_dir, 
	      n_iterations,
	      dilution_factor,
	      restart_training_from, 
	      node_vec, 
	      input_variable_info, 
	      flavor_weights_struct, 
	      n_training_events_target); 
    }
    catch (const LoadReducedDSException& e){ 
      PyErr_SetString(PyExc_IOError,"could not load dataset"); 
      return NULL; 
    }
    catch (const WriteFileException& e){ 
      PyErr_SetString(PyExc_IOError,"could not write output"); 
      return NULL; 
    }
    catch (const NNException& e) { 
      PyErr_SetString(PyExc_StandardError,"generic nn exception"); 
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
    catch (const LoadNetworkException& e){ 
      PyErr_SetString(PyExc_IOError,"could not load network"); 
      return NULL; 
    }
    catch (const WriteFileException& e){ 
      PyErr_SetString(PyExc_IOError,"could not write output"); 
      return NULL; 
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

  }

  Py_INCREF(Py_None);

  return Py_None;
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
    catch (const LoadNetworkException& e){ 
      PyErr_SetString(PyExc_IOError,"could not load network"); 
      return NULL; 
    }
    catch (const WriteFileException& e){ 
      PyErr_SetString(PyExc_IOError,"could not write output"); 
      return NULL; 
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
