#include <Python.h>
#include "TString.h"

#include "trainNN.hh"

static const char* doc_string = 
  "run the neural net. \nKeywords:\ninput_file\noutput_class\n\
n_iterations\ndilution_factor\nuse_sd\nwith_ip3d\n\
nodes_first_layer\nnodes_second_layer\nrestart_training_from\ndebug"; 

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

  static const char *kwlist[] = {"input_file", "output_class", "n_iterations", 
				 "dilution_factor","use_sd","with_ip3d",
				 "nodes_first_layer","nodes_second_layer", 
				 "restart_training_from", "debug", NULL};
 
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
    trainNN_no_root(input_file, 
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

const char* getDocString(const char* kw_list[], int n_entries){ 
  std::string doc = "run the neural net, inputs:\n"; 
  for (int entry_n = 0; entry_n < n_entries; entry_n++){ 
    doc.append(kw_list[entry_n]);
    doc.append("\n"); 
  }
  return doc.c_str(); 
}


 static PyMethodDef keywdarg_methods[] = {
  // The cast of the function is necessary since PyCFunction values
  // only take two PyObject* parameters, and keywdarg() takes
  // three.
  {"trainNN", (PyCFunction)train_py, 
   METH_VARARGS | METH_KEYWORDS,
   doc_string},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" PyMODINIT_FUNC initpynn(void)
{
  Py_InitModule("pynn", keywdarg_methods);
}

