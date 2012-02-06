// python wrapper for JetNet based training algorithm
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include "trainNN.hh"
#include "testNN.hh"
#include "nnExceptions.hh"
#include "makeTestNtuple.hh"
#include "pynn.hh"


static const char* train_doc_string = 
  "run the neural net. \n"
  "Keywords:\n"
  "reduced_dataset\n"
  "output_directory\n"
  "n_iterations\n"
  "dilution_factor\n"
  "use_sd\n"
  "with_ip3d\n"
  "nodes_first_layer\n"
  "nodes_second_layer\n"
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
  bool use_sd = false; 
  bool with_ip3d = true; 
  PyObject* nodes = 0; 
  int restart_training_from = 0; 
  bool debug = false; 

  const char *kwlist[] = {
    "reduced_dataset",
    "output_directory", 
    "n_iterations", 
    "dilution_factor",
    "use_sd",
    "with_ip3d",
    "nodes"
    "restart_training_from",
    "debug", 
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords
      (args, keywds, "s|siibbOib", 
       // this function should take a const, and 
       // may be changed, until then we'll cast
       const_cast<char**>(kwlist),
       &input_file, 
       &output_dir, 
       &n_iterations, &dilution_factor, 
       &use_sd, &with_ip3d, 
       &nodes, 
       &restart_training_from, 
       &debug)
      ) return NULL;

  std::vector<int> node_vec; 
  try {
    node_vec = parse_int_tuple(nodes); 
  }
  catch(ParseException e) { 
    PyErr_SetString(PyExc_TypeError,
		    "expected a tuple of int, found something else"); 
    return 0;
  }

  if (debug){ 
    printf("in = %s, out dir = %s, itr = %i, rest from = %i\n", 
  	   input_file, output_dir, n_iterations, restart_training_from); 
  }

  if (node_vec.size() != 2){ 
    PyErr_SetString(PyExc_ValueError,
		    "node tuple must have two entries (for now)"); 
    return 0;
  }
  int nodes_first_layer = node_vec.at(0); 
  int nodes_second_layer = node_vec.at(1); 

  
  trainNN(input_file, 
	  output_dir, 
	  n_iterations,
	  dilution_factor,
	  use_sd,
	  with_ip3d,
	  nodes_first_layer,
	  nodes_second_layer,
	  restart_training_from);
  

  Py_INCREF(Py_None);

  return Py_None;
}

static const char* test_doc_string = 
  "test the neural net. \n"
  "Keywords:\n"
  "input_file\n"
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
    "input_file",
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
    catch (LoadNetworkException e){ 
      PyErr_SetString(PyExc_IOError,"could not load network"); 
      return NULL; 
    }
    catch (WriteFileException e){ 
      PyErr_SetString(PyExc_IOError,"could not write output"); 
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
 
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "sss|sb", 
				   // this function should take a const, and 
				   // may be changed, until then we'll cast
				   const_cast<char**>(kwlist),
				   &input_weights_name,
				   &input_dataset_name, 
				   &output_file_name, 
				   &output_tree_name, 
				   &debug))
    return NULL;

  if (debug){ 
    printf("in wt = %s, in ds = %s, out file = %s, out tree = %s\n", 
  	   input_weights_name, input_dataset_name, 
	   output_file_name, output_tree_name); 
  }

  else{ 
    makeTestNtuple(input_weights_name, 
    		   input_dataset_name,
    		   output_file_name, 
    		   output_tree_name); 
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




std::vector<int> parse_int_tuple(PyObject* py_list){ 
  std::vector<int> ints; 
  if (py_list == 0) { 
    return ints; 
  }

  bool ok = PyTuple_Check(py_list); 
  if (!ok) {
    throw TupleParseException(); 
  }

  int n_items = PyTuple_Size(py_list); 
  for (int i = 0; i < n_items; i++){ 
    PyObject* the_ob = PyTuple_GetItem(py_list, i); 
    if (!PyInt_Check(the_ob)){ 
      throw IntParseException(); 
    }

    int the_int = PyInt_AsLong(the_ob); 
    ints.push_back(the_int); 
  }
  return ints; 

}
