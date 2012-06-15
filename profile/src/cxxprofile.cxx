// python wrapper for neural net training set preperation 
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <stdexcept>

#include "cxxprofile.hh"
#include "profile_fast.hh"
#include "profile_2d.hh"
  
static const char* doc_string = 
  "profile_fast"
  "(in_file, tree, out_file, ints, doubles, tags, "
  "max_entries, show_progress) "
  "--> n_passed, n_failed\n"
  "Builds file 'output'.\n"
  "'ints' and 'doubles' take a list of tuples (name, n_bins, min, max).\n"
  "If not given, n_bins defaults to (max - min + 1) for int, "
  "(n_entries / 100) for doubles";

PyObject* py_profile_fast(PyObject *self, 
			  PyObject *args, 
			  PyObject *keywds)
{
  const char* file_name; 
  const char* tree_name; 
  const char* output_file; 
  PyObject* int_leaves = 0; 
  PyObject* double_leaves = 0; 
  PyObject* tag_leaves = 0; 
  int max_entries = -1; 
  bool show_progress = false; 
  const char* kwlist[] = {
    "in_file",
    "tree", 
    "out_file", 
    "ints", 
    "doubles", 
    "tags",
    "max_entries",
    "show_progress", 
    NULL};
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     "sss|OOOib", 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(kwlist),
     &file_name,
     &tree_name, 
     &output_file, 
     &int_leaves, 
     &double_leaves, 
     &tag_leaves, 
     &max_entries, 
     &show_progress); 

  if (!ok) return NULL;

  std::vector<LeafInfo> int_leaf_vec = build_leaf_info_vec(int_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<LeafInfo> double_leaf_vec = build_leaf_info_vec(double_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<std::string> tag_leaves_vec = build_string_vec(tag_leaves); 
  if (PyErr_Occurred()) return NULL; 

  unsigned options = opt::def_opt; 
  if (show_progress) options |= opt::show_progress; 

  std::pair<int,int> n_pass_fail(std::make_pair(0,0)); 
  try { 
    n_pass_fail = profile_fast(file_name, tree_name, 
			       int_leaf_vec, double_leaf_vec, tag_leaves_vec, 
			       output_file, 
			       max_entries, 
			       options); 
  }
  catch (const std::runtime_error& e) { 
    PyErr_SetString(PyExc_IOError,e.what()); 
    return 0; 
  }
  
  return Py_BuildValue("ii",n_pass_fail.first, n_pass_fail.second); 

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

std::vector<LeafInfo> build_leaf_info_vec(PyObject* list)
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
      PyErr_SetString(PyExc_IOError,"leaf info takes a tuple"); 
      return info; 
    }
    LeafInfo i = build_leaf_info(info_tuple); 
    if (PyErr_Occurred() ) return info; 
    info.push_back(i); 
  }
  return info; 
} 

LeafInfoPairs build_plot2d_vec(PyObject* list) 
{ 
  LeafInfoPairs info_pairs_vec; 
  if (!list) return info_pairs_vec; 
  int n_obj = PyList_Size(list); 
  if (PyErr_Occurred()) {
    PyErr_SetString(PyExc_IOError,"leaf info takes a tuple pair list"); 
    return info_pairs_vec; 
  }
  for (int list_index = 0; list_index < n_obj; list_index++){ 
    PyObject* info_pair = PyList_GetItem(list,list_index); 
    int n_infos = PySequence_Size(info_pair); 
    if (PyErr_Occurred() || n_infos != 2 ) { 
      PyErr_SetString(PyExc_IOError,"leaf info list value takes "
		      "a tuple pair"); 
      return info_pairs_vec; 
    }

    PyObject* first_info = PySequence_GetItem(info_pair,0); 
    LeafInfo i = build_leaf_info(first_info); 
    if (PyErr_Occurred() ) return info_pairs_vec; 

    PyObject* second_info = PySequence_GetItem(info_pair,1); 
    LeafInfo j = build_leaf_info(second_info); 
    if (PyErr_Occurred() ) return info_pairs_vec; 
    info_pairs_vec.push_back(std::make_pair(i,j)); 
  }
  return info_pairs_vec;  
  
}


LeafInfo build_leaf_info(PyObject* info_tuple) { 
  LeafInfo i; 
  int tup_size = PyTuple_Size(info_tuple); 
  if (tup_size < 3 || tup_size > 4) { 
    PyErr_SetString(PyExc_IOError,"leaf tuple must have 3 or 4 values:"
		    "(name, n_bin, min, max) or (name, min, max)"); 
    return i; 
  }

  PyObject* name = PyTuple_GetItem(info_tuple, 0); 
  if (!PyString_Check(name)){
    PyErr_SetString(PyExc_IOError,
		    "first leaf tuple values must be a string"); 
    return i; 
  }
  i.name = PyString_AsString(name); 

  for (int n = 1; n < tup_size; n++){ 
    PyObject* number = PyTuple_GetItem(info_tuple,n); 
    if (!PyNumber_Check(number)) { 
      PyErr_SetString(PyExc_IOError,"leaf tuple values must be numbers"); 
      return i; 
    }
  }

  PyObject* min = 0; 
  PyObject* max = 0; 
  if (tup_size == 4) { 
    PyObject* n_bins = PyTuple_GetItem(info_tuple, 1); 
    if (!PyInt_Check(n_bins) || PyInt_AsLong(n_bins) < 1 ){ 
      PyErr_SetString(PyExc_IOError,"n_bins must be an int > 1"); 
      return i; 
    }
    i.n_bins = PyInt_AsLong(n_bins); 
    min = PyTuple_GetItem(info_tuple,2); 
    max = PyTuple_GetItem(info_tuple,3); 
  }
  else { 
    min = PyTuple_GetItem(info_tuple,1); 
    max = PyTuple_GetItem(info_tuple,2); 
    if (PyInt_Check(min) && PyInt_Check(max) ) { 
      i.n_bins = -2; 		// -2 = calculate from range
    }
    else { 
      i.n_bins = -1; 		// -1 = calculate from entries
    }
  }

  i.min = PyFloat_AsDouble(min); 
  i.max = PyFloat_AsDouble(max); 
  return i; 
}

// void dummy() { 
//   FilterHist(1,0,1,new double, new int); 
// }
