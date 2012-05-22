// python wrapper for neural net training set preperation 
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <stdexcept>

#include "cxxprofile.hh"
#include "profile_fast.hh"
  
static const char* doc_string = 
  "profile_fast(file, tree, ints, doubles, output, max_entries, n_bins)"
  " --> n_passed"
  "\nbuilds file 'output', returns the number of events which passed"; 

PyObject* py_profile_fast(PyObject *self, 
			  PyObject *args, 
			  PyObject *keywds)
{
  const char* file_name; 
  const char* tree_name; 
  PyObject* int_leaves = 0; 
  PyObject* double_leaves = 0; 
  PyObject* tag_leaves = 0; 
  const char* output_file = "out.root"; 
  int max_entries = -1; 
  int n_bins = 500; 
  const char* kwlist[] = {
    "in_file",
    "tree", 
    "ints", 
    "doubles", 
    "tags",
    "output", 
    "max_entries",
    "n_bins", 
    NULL};
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     "ss|OOOsii", 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(kwlist),
     &file_name,
     &tree_name, 
     &int_leaves, 
     &double_leaves, 
     &tag_leaves, 
     &output_file, 
     &max_entries, 
     &n_bins); 

  if (!ok) return NULL;

  std::vector<LeafInfo> int_leaf_vec = build_leaf_info(int_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<LeafInfo> double_leaf_vec = build_leaf_info(double_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<std::string> tag_leaves_vec = build_string_vec(tag_leaves); 
  if (PyErr_Occurred()) return NULL; 

  int n_entries = 0; 
  try { 
    n_entries = profile_fast(file_name, tree_name, 
			     int_leaf_vec, double_leaf_vec, tag_leaves_vec, 
			     output_file, 
			     max_entries, n_bins); 
  }
  catch (const std::runtime_error& e) { 
    PyErr_SetString(PyExc_IOError,e.what()); 
    return 0; 
  }
  
  return Py_BuildValue("i",n_entries); 

  // --- make sure you call INCREF if you return Py_None
  // Py_INCREF(Py_None);
  // return Py_None;
}



static PyMethodDef keywdarg_methods[] = {
  // The cast of the function is necessary since PyCFunction values
  // only take two PyObject* parameters, and keywdarg() takes
  // three.
  {"profile_fast", (PyCFunction)py_profile_fast, 
   METH_VARARGS | METH_KEYWORDS,
   doc_string},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 

  PyMODINIT_FUNC initcxxprofile(void)
  {
    Py_InitModule("cxxprofile", keywdarg_methods);
  }

}

std::vector<std::string> build_string_vec(PyObject* list) 
{
  std::vector<std::string> out; 
  if (!list) { 
    return out; 
  }
  int n_obj = PyList_Size(list); 
  if (PyErr_Occurred()) {
    PyErr_SetString(PyExc_IOError,"tags takes a list"); 
    return out; 
  }
  for (int n = 0; n < n_obj; n++) { 
    PyObject* the_obj = PyList_GetItem(list, n); 
    std::string the_string = PyString_AsString(the_obj); 
    if (PyErr_Occurred()) { 
      PyErr_SetString(PyExc_IOError,"tags must be a list of strings"); 
      return out; 
    }
    out.push_back(the_string); 
  }
  return out; 
}

std::vector<LeafInfo> build_leaf_info(PyObject* list)
{
  std::vector<LeafInfo> info; 
  if (!list) { 
    return info; 
  }
  int n_obj = PyList_Size(list); 
  if (PyErr_Occurred()) {
    PyErr_SetString(PyExc_IOError,"leaf info takes a tuple list"); 
    return info; 
  }
  for (int list_index = 0; list_index < n_obj; list_index++){ 
    PyObject* info_tuple = PyList_GetItem(list,list_index); 
    if (!PyTuple_Check(info_tuple)) { 
      PyErr_SetString(PyExc_IOError,"leaf info takes a tuple list"); 
      return info; 
    }
    if (PyTuple_Size(info_tuple) != 3) { 
      PyErr_SetString(PyExc_IOError,"leaf tuple must have three values"); 
      return info; 
    }
    PyObject* name = PyTuple_GetItem(info_tuple, 0); 
    PyObject* min = PyTuple_GetItem(info_tuple, 1); 
    PyObject* max = PyTuple_GetItem(info_tuple, 2); 

    if (!PyString_Check(name) ||
	!PyNumber_Check(min) ||
	!PyNumber_Check(max) ) { 
      PyErr_SetString(PyExc_IOError,"leaf tuple values must be "
		      "(string, float, float)"); 
      return info; 
    }
    LeafInfo i; 
    i.name = PyString_AsString(name); 
    i.min = PyFloat_AsDouble(min); 
    i.max = PyFloat_AsDouble(max); 
    info.push_back(i); 
  }
  return info; 
} 

// void dummy() { 
//   FilterHist(1,0,1,new double, new int); 
// }
