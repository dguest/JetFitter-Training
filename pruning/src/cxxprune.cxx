// python functions to prune trees
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <stdexcept>
// #include <algorithm> // sort

#include "prune_tree.hh"
  
static const char* doc_string = 
  "simple_prune"
  "(in_file, tree, out_file, int_cuts, double_cuts, "
  "max_entries, verbose) "
  "--> TBD\n"
  "Builds file 'out_file'.\n"; 

PyObject* py_simple_prune(PyObject *self, 
			  PyObject *args, 
			  PyObject *keywds)
{
  const char* file_name; 
  const char* tree_name; 
  const char* output_file; 
  PyObject* py_int_cuts = 0; 
  PyObject* py_double_cuts = 0; 
  int max_entries = -1; 
  bool verbose = false; 
  const char* kwlist[] = {
    "in_file",
    "tree", 
    "out_file", 
    "int_cuts", 
    "double_cuts", 
    "max_entries",
    "verbose", 
    NULL};
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     "sss|OOib:simple_prune", 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(kwlist),
     &file_name,
     &tree_name, 
     &output_file, 
     &py_int_cuts, 
     &py_double_cuts, 
     &max_entries, 
     &verbose); 

  if (!ok) return NULL;

  unsigned options = 0
  if (verbose) options |= opt::verbose; 

  std::vector<SubTreeIntInfo> int_cuts; 
  int n_int_cuts = PyList_Size(py_int_cuts); 
  if (PyErr_Occurred()) { 
    return NULL; 
  }
  for (int cut_n = 0; cut_n < n_int_cuts; cut_n++) { 
    SubTreeIntInfo info; 
    PyObject* py_cut = PyList_GetItem(py_int_cuts, cut_n); 
    if (!PySequence_Size(py_cut) != 2) { 
      PyErr_SetString("expect a list of tuples (name, value) for int_cuts"); 
      return NULL; 
    }
    PyObject* py_name = PySequence_GetItem(py_cut, 0); 
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString("expect tuples (name, value) for int_cuts, name"
		      " must be a string"); 
      return NULL; 
    }
    PyObject* py_value = PySequence_GetItem(py_cut, 1); 
    info.value = PyInt_AsLong(py_value); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString("expect tuples (name, value) for int_cuts, value"
		      " must be an int"); 
      return NULL; 
    }
    int_cuts.push_back(info); 
  }

  std::vector<SubTreeDoubleInfo> double_cuts; 
  int n_double_cuts = PyList_Size(py_double_cuts); 
  if (PyErr_Occurred()) { 
    return NULL; 
  }
  for (int cut_n = 0; cut_n < n_double_cuts; cut_n++) { 
    SubTreeDoubleInfo info; 
    PyObject* py_cut = PyList_GetItem(py_double_cuts, cut_n); 
    if (!PySequence_Size(py_cut) != 3) { 
      PyErr_SetString("expect a list of tuples (name, low, high)"
		      " for double_cuts"); 
      return NULL; 
    }
    PyObject* py_name = PySequence_GetItem(py_cut, 0); 
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString("expect tuples (name, low, high) for double_cuts, name"
		      " must be a string"); 
      return NULL; 
    }
    PyObject* py_low = PySequence_GetItem(py_cut, 1); 
    info.low = PyFloat_AsDouble(py_low); 
    PyObject* py_high = PySequence_GetItem(py_cut, 2); 
    info.high = PyFloat_AsDouble(py_high); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString("expect tuples (name, low, high) for double_cuts"
		      " high and low must be floats"); 
      return NULL; 
    }
    double_cuts.push_back(info); 
  }

  int return_code; 
  try { 
    return_code = simple_prune(file_name, tree_name, 
			       int_cuts, double_cuts, 
			       output_file, 
			       max_entries, 
			       options); 
  }
  catch (const std::runtime_error& e) { 
    PyErr_SetString(PyExc_IOError,e.what()); 
    return 0; 
  }
  
  return Py_BuildValue("i",return_code); 

  // --- make sure you call INCREF if you return Py_None
  // Py_INCREF(Py_None);
  // return Py_None;
}




static PyMethodDef keywdarg_methods[] = {
  // The cast of the function is necessary since PyCFunction values
  // only take two PyObject* parameters, and keywdarg() takes
  // three.
  {"simple_prune", (PyCFunction)py_simple_prune, 
   METH_VARARGS | METH_KEYWORDS,
   doc_string},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 

  PyMODINIT_FUNC initcxxprune(void)
  {
    Py_InitModule("cxxprune", keywdarg_methods);
  }

}

