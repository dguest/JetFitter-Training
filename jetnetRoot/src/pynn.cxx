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
#include "NNFileConverters.hh"
#include "pyparse.hh"
#include "makeTestNtuple.hh"
#include "pynn.hh"

static const std::string train_name = "trainNN"; 

static const std::string train_additional = 
  "flag values:"
  "\n\td: debug,"
  "\n\tg: giacintos training,"
  "\n\tt: throw on warning,"
  "\n\tv: verbose,"
  "\n\tr: random entry selection (default takes first entries),"
  "\n\ts: shuffle training entries"; 

static const char *train_kwlist[] = {
  "reduced_dataset",
  "output_directory", 
  "n_iterations", 
  "normalization", 
  "nodes", 
  "restart_training_from",
  "flavor_weights", 
  "n_training_events_target", 
  "n_patterns_per_update", 
  "learning_rate", 
  "learning_rate_decrease", 
  "flags", 
  NULL};

static const std::string train_argtypes = "s|siOOiOiiffs"; 

static char train_doc[MAX_DOC_STRING_LENGTH]; 


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
  const char* flags = ""; 

  inputs.n_patterns_per_update = N_PATTERNS_PER_UPDATE; 
  inputs.learning_rate = LEARNING_RATE; 
  inputs.learning_rate_decrease = LEARNING_RATE_DECREASE; 

  std::string argtype_str = train_argtypes + ":" + train_name; 
 
  if (!PyArg_ParseTupleAndKeywords
      (args, keywds, argtype_str.c_str(), 
       // this function should take a const, and 
       // may be changed, until then we'll cast
       const_cast<char**>(train_kwlist),
       &input_file, 
       &output_dir, 
       &inputs.n_iterations, 
       &normalization, 
       &nodes, 
       &inputs.restart_training_from, 
       &flavor_weights, 
       &inputs.n_training_events, 
       &inputs.n_patterns_per_update, 
       &inputs.learning_rate, 
       &inputs.learning_rate_decrease, 
       &flags)
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
  flavor_weights_map["light"] = 1; 

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
    if (!flavor_weights_map.count(itr->first) ) { 
      std::string prob_str = "not sure what '" + itr->first + "' is as a "
	"flavor weight"; 
      PyErr_SetString(PyExc_TypeError, prob_str.c_str()); 
    }
    flavor_weights_map[itr->first] = itr->second; 
  }

  FlavorWeights flavor_weights_struct; 
  flavor_weights_struct.bottom = flavor_weights_map["bottom"]; 
  flavor_weights_struct.charm = flavor_weights_map["charm"]; 
  flavor_weights_struct.light = flavor_weights_map["light"]; 

  // --- parse flags
  unsigned bit_flags = 0; 
  if (strchr(flags,'g')) bit_flags |= train::giacintos;
  if (strchr(flags,'t')) bit_flags |= train::throw_on_warn;
  if (strchr(flags,'v')) bit_flags |= train::verbose;
  if (strchr(flags,'r')) bit_flags |= train::use_random_entries; 
  if (strchr(flags,'s')) bit_flags |= train::shuffle_train_set; 

  bool debug = strchr(flags,'d'); 
  

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
      trainNN(inputs, node_vec, input_variable_info, 
	      flavor_weights_struct, bit_flags); 
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

static const std::string augment_name = "augment_tree"; 
static const char* augment_kwlist[] = {
  "in_file",
  "nn_file", 
  "tree", 
  "out_file", 
  "ints", 
  "doubles", 
  "subset", 
  "extension", 
  "max_entries",
  "start_entry", 
  "show_progress", 
  NULL};
static const std::string augment_argtypes = "ss|ssOOOsiib"; 
static char augment_doc[MAX_DOC_STRING_LENGTH]; 


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
  int start_entry = 0; 
  bool show_progress = false; 

  std::string argtypes_string = augment_argtypes + ":" + augment_name; 

  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     argtypes_string.c_str(), 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(augment_kwlist),
     &file_name,
     &nn_file, 
     &tree_name, 
     &output_file, 
     &int_leaves, 
     &double_leaves, 
     &py_subset, 
     &extension, 
     &max_entries, 
     &start_entry, 
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
       start_entry, 
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


// ------------ makehist ------------------

static const std::string makehist_name = "makehist"; 
static const char* makehist_kwlist[] = {
  "nn_file", 
  "out_file", 
  "nn_name", 
  NULL};
static const std::string makehist_argtypes = "ss|s"; 
static char makehist_doc[MAX_DOC_STRING_LENGTH]; 


PyObject* py_makehist(PyObject *self, 
		      PyObject *args, 
		      PyObject *keywds) 
{
  const char* nn_file; 
  const char* out_file; 
  const char* nn_name = "TFlavorNetwork"; 
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, makehist_argtypes.c_str(), 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(makehist_kwlist),
     &nn_file, 
     &out_file, 
     &nn_name); 

  nn_file_to_hist_file(nn_file, out_file, nn_name); 
  
  return Py_BuildValue(""); 

}


static PyMethodDef keywdarg_methods[] = {
  // The cast of the function is necessary since PyCFunction values
  // only take two PyObject* parameters, and keywdarg() takes
  // three.
  {train_name.c_str(), (PyCFunction)train_py, 
   METH_VARARGS | METH_KEYWORDS,
   train_doc},
  {"testNN", (PyCFunction)test_py, 
   METH_VARARGS | METH_KEYWORDS,
   test_doc_string},
  {"makeNtuple", (PyCFunction)make_test_ntuple, 
   METH_VARARGS | METH_KEYWORDS,
   ntuple_doc_string},
  {augment_name.c_str(), (PyCFunction)py_augment_tree, 
   METH_VARARGS | METH_KEYWORDS,
   augment_doc},
  {makehist_name.c_str(), (PyCFunction)py_makehist, 
   METH_VARARGS | METH_KEYWORDS,
   makehist_doc},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" PyMODINIT_FUNC initpynn(void)
{
  build_doc(train_doc, 
	    train_name + "(", train_kwlist, ")\n" + train_additional); 
  build_doc(augment_doc, 
	    augment_name + "(", augment_kwlist, ")"); 
  build_doc(makehist_doc, makehist_name + "(", makehist_kwlist, ")"); 
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


void build_doc(char* doc_array, 
	       std::string b, const char** input_kwds, std::string a){ 
  strcat(doc_array, b.c_str()); 
  size_t n_unwraped_cols = 0; 
  for (int n = 0; n < 20; n++) { 
    const char* this_str = input_kwds[n]; 
    if (! this_str) break; 
    if (n != 0) strcat(doc_array,", "); 
    size_t n_cols = strlen(doc_array) - n_unwraped_cols; 
    if (n_cols > 80) { 
      strcat(doc_array,"\n\t"); 
      n_unwraped_cols = n_cols; 
    }
    strcat(doc_array, this_str); 
  }
  strcat(doc_array, a.c_str()); 
  assert(strlen(doc_array) < MAX_DOC_STRING_LENGTH); 
}
