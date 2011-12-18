// python wrapper for JetNet based training algorithm
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include "TString.h"

#include "trainNN.hh"
#include "testNN.hh"

static const char* train_doc_string = 
  "run the neural net. \n"
  "Keywords:\n"
  "input_file\n"
  "output_class\n"
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
  const char* output_class = "JetFitterNN"; 
  int n_iterations = 10; 
  int dilution_factor = 2; 
  bool use_sd = false; 
  bool with_ip3d = true; 
  int nodes_first_layer = 10; 
  int nodes_second_layer = 9; 
  int restart_training_from = 0; 
  bool debug = false; 

  static const char *kwlist[] = {
    "input_file",
    "output_class", 
    "n_iterations", 
    "dilution_factor",
    "use_sd",
    "with_ip3d",
    "nodes_first_layer",
    "nodes_second_layer", 
    "restart_training_from",
    "debug", 
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|siibbiiib", 
				   // this function should take a const, and 
				   // may be changed, until then we'll cast
				   const_cast<char**>(kwlist),
				   &input_file, &output_class, 
				   &n_iterations, &dilution_factor, 
				   &use_sd, &with_ip3d, &nodes_first_layer, 
				   &nodes_second_layer, 
				   &restart_training_from, 
				   &debug))
    return NULL;

  if (debug){ 
    printf("in = %s, out = %s, itr = %i, rest from = %i\n", 
  	   input_file, output_class, n_iterations, restart_training_from); 
  }

  else{ 
    trainNN(input_file, 
	    output_class,
	    n_iterations,
	    dilution_factor,
	    use_sd,
	    with_ip3d,
	    nodes_first_layer,
	    nodes_second_layer,
	    restart_training_from);
  }

  Py_INCREF(Py_None);

  return Py_None;
}

static const char* test_doc_string = 
  "test the neural net. \n"
  "Keywords:\n"
  "input_file\n"
  "training_file\n"
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
  bool use_sd = false; 
  bool with_ip3d = true; 

  static const char *kwlist[] = {
    "input_file",
    "training_file", 
    "dilution_factor",
    "use_sd",
    "with_ip3d",
    NULL};
 
  if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|sibb", 
				   // this function should take a const, and 
				   // may be changed, until then we'll cast
				   const_cast<char**>(kwlist),
				   &training_file,
				   &dilution_factor, 
				   &use_sd, &with_ip3d))
    return NULL;

  testNN(input_file, 
	 training_file,
	 dilution_factor,
	 use_sd,
	 with_ip3d); 

  Py_INCREF(Py_None);

  return Py_None;
}



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
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" PyMODINIT_FUNC initpynn(void)
{
  Py_InitModule("pynn", keywdarg_methods);
}

