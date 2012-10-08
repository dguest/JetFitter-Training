// python wrapper for neural net training set preperation 
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <stdexcept>
#include <algorithm> // sort

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
     "sss|OOOib:profile_fast", 
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

static const char* doc_2d = 
  "pro2d"
  "(in_file, tree, out_file, plots, tags, "
  "max_entries, show_progress) "
  "--> out file name, hist list\n"
  "Builds file 'out_file'.\n"
  "'plots' takes a list of pairs of leaf tuples. \n"
  "\n"
  "Leaf tuples are expected in one of two forms:\n"
  "1) (name, n_bins, min, max) --- "
  "if n_bins is ommitted will guess based on min and max\n"
  "2) (name, bin_bounds) --- for variable bin sizes\n"
  "optionally, a leaf for weight can be given after 'name'\n"; 

PyObject* py_pro2d(PyObject *self, 
		   PyObject *args, 
		   PyObject *keywds)
{
  const char* file_name; 
  const char* tree_name; 
  PyObject* plots = 0; 
  PyObject* tag_leaves = 0; 
  PyObject* py_masks = 0; 
  const char* output_file; 
  int max_entries = -1; 
  bool show_progress = false; 
  const char* kwlist[] = {
    "in_file",
    "tree", 
    "out_file", 
    "plots", 
    "tags",
    "masks", 
    "max_entries",
    "show_progress", 
    NULL};
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     "sssO|OOib:pro2d", 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(kwlist),
     &file_name,
     &tree_name, 
     &output_file, 
     &plots, 
     &tag_leaves, 
     &py_masks, 
     &max_entries, 
     &show_progress); 

  if (!ok) return NULL;

  LeafInfoPairs leaf_info_pairs = build_plot2d_vec(plots); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<TagInfo> tag_leaves_vec = build_tag_vec(tag_leaves); 
  if (PyErr_Occurred()) return NULL; 
  std::vector<MaskInfo> masks = build_mask_vec(py_masks); 
  if (PyErr_Occurred()) return NULL; 

  unsigned options = opt::def_opt; 
  if (show_progress) options |= opt::show_progress; 

  ProfileInfo profile_return; 
  try { 
    profile_return = pro_2d(file_name, 
			    tree_name, 
			    leaf_info_pairs, 
			    tag_leaves_vec, 
			    masks, 
			    output_file, 
			    max_entries, 
			    options); 
  }
  catch (const std::runtime_error& e) { 
    PyErr_SetString(PyExc_IOError,e.what()); 
    return 0; 
  }

  PyObject* hist_list = PyList_New(0); 
  for (std::vector<std::string>::const_iterator 
	 hist_name_itr = profile_return.hist_names.begin(); 
       hist_name_itr != profile_return.hist_names.end(); 
       hist_name_itr++) { 
    PyList_Append(hist_list, PyString_FromString(hist_name_itr->c_str())); 
  }
  return Py_BuildValue("sN",profile_return.file_name.c_str(), hist_list); 

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
  {"pro2d", (PyCFunction)py_pro2d, 
   METH_VARARGS | METH_KEYWORDS,
   doc_2d},
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
  if (tup_size < 2 || tup_size > 5) { 
    PyErr_SetString(PyExc_IOError,"leaf tuple must be of form:"
		    "(name, [weight_name, n_bins,] min, max)"
		    "or (name, [weight_name,] bin_div_list)"); 
    return i; 
  }

  int tuple_index = 0; 

  PyObject* name = PyTuple_GetItem(info_tuple, tuple_index); 
  if (!PyString_Check(name)){
    PyErr_SetString(PyExc_IOError,
		    "first leaf tuple values must be a string"); 
    return i; 
  }
  i.name = PyString_AsString(name); 
  tuple_index++; 

  PyObject* wt_name = PyTuple_GetItem(info_tuple, tuple_index); 
  if (PyString_Check(wt_name) ) { 
    i.wt_name = PyString_AsString(wt_name); 
    tuple_index++; 
  }
  else { 
    i.wt_name = ""; 
  }

  if (PyList_Check(PyTuple_GetItem(info_tuple, tuple_index))) { 
    if (tup_size != tuple_index + 1) { 
      PyErr_SetString(PyExc_IOError,"found list as non-terminal entry in "
		      "leaf info tuple"); 
      return i; 
    }
    PyObject* bins_list = PyTuple_GetItem(info_tuple,tuple_index); 
    i.bin_bounds = build_double_vec(bins_list); 
    std::sort(i.bin_bounds.begin(), i.bin_bounds.end()); 
    i.n_bins = i.bin_bounds.size() - 1; 
    return i; 
  }

  // make sure everything else is a number 
  for (int n = tuple_index; n < tup_size; n++){ 
    PyObject* number = PyTuple_GetItem(info_tuple,n); 
    if (!PyNumber_Check(number)) { 
      PyErr_SetString(PyExc_IOError,"leaf tuple values must be numbers"); 
      return i; 
    }
  }

  PyObject* min = 0; 
  PyObject* max = 0; 
  if (tup_size - tuple_index == 3) { 
    PyObject* n_bins = PyTuple_GetItem(info_tuple, tuple_index); 
    tuple_index++; 
    if (!PyInt_Check(n_bins) || PyInt_AsLong(n_bins) < 1 ){ 
      PyErr_SetString(PyExc_IOError,"n_bins must be an int > 1"); 
      return i; 
    }
    i.n_bins = PyInt_AsLong(n_bins); 
    min = PyTuple_GetItem(info_tuple, tuple_index); 
    tuple_index++; 
    max = PyTuple_GetItem(info_tuple, tuple_index); 
  }
  else { 
    assert(tup_size - tuple_index == 2); 
    min = PyTuple_GetItem(info_tuple, tuple_index); 
    tuple_index++; 
    max = PyTuple_GetItem(info_tuple, tuple_index); 
    tuple_index++; 
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

std::vector<double> build_double_vec(PyObject* list) { 
  std::vector<double> vec; 
  int n_entries = PyList_Size(list); 
  for (int n = 0; n < n_entries; n++) { 
    PyObject* py_double = PyList_GetItem(list, n); 
    double the_double = PyFloat_AsDouble(py_double); 
    if (PyErr_Occurred()) { 
      return vec; 
    }
    vec.push_back(the_double); 
  }
  return vec; 
}

std::vector<MaskInfo> build_mask_vec(PyObject* list) { 
  std::vector<MaskInfo> vec; 
  if (!list) return vec; 
  int n_entries = PyList_Size(list); 
  if (PyErr_Occurred() ) return vec; 
  for (int n = 0; n < n_entries; n++) { 
    PyObject* py_mask = PyList_GetItem(list, n); 

    if (!PyDict_Check(py_mask) ) { 
      PyErr_SetString(PyExc_IOError,"mask should be a list of dicts"); 
      return vec; 
    }

    MaskInfo info; 
    PyObject* py_name = PyDict_GetItemString(py_mask, "name"); 
    if (!py_name) { 
      PyErr_SetString(PyExc_IOError,"no 'name' in mask dict"); 
      return vec; 
    }
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) return vec; 
    PyObject* py_leaf_name = PyDict_GetItemString(py_mask, "leaf_name"); 
    if (!py_leaf_name) { 
      PyErr_SetString(PyExc_IOError,"no 'leaf_name' in mask dict"); 
      return vec; 
    }
    info.leaf_name = PyString_AsString(py_leaf_name); 
    if (PyErr_Occurred() ) return vec; 
    PyObject* py_accept = PyDict_GetItemString(py_mask, "accept_mask"); 
    if (!py_accept) { 
      PyErr_SetString(PyExc_IOError,"no 'accept_mask' in mask dict"); 
      return vec; 
    }
    info.accept_mask = PyLong_AsUnsignedLongMask(py_accept); 
    if (PyErr_Occurred() ) return vec; 
    info.veto_mask = 0; 
    PyObject* py_veto = PyDict_GetItemString(py_mask, "veto_mask"); 
    if (py_veto) { 
      info.veto_mask = PyLong_AsUnsignedLongMask(py_veto); 
      if (PyErr_Occurred() ) return vec; 
    }
    vec.push_back(info); 
    
  }
  return vec; 
}

std::vector<TagInfo> build_tag_vec(PyObject* list) { 
  std::vector<TagInfo> vec; 
  if (!list) return vec; 
  int n_entries = PyList_Size(list); 
  if (PyErr_Occurred() ) return vec; 
  for (int n = 0; n < n_entries; n++) { 
    PyObject* py_mask = PyList_GetItem(list, n); 

    if (!PyDict_Check(py_mask) ) { 
      std::vector<std::string> old_style_tags = build_string_vec(list); 
      if (PyErr_Occurred()) return vec; 
      for (std::vector<std::string>::const_iterator 
	     itr = old_style_tags.begin(); 
	   itr != old_style_tags.end(); itr++) { 
	TagInfo info; 
	info.name = *itr; 
	info.leaf_name = *itr; 
	info.value = 1; 
      }
      return vec; 
    }

    TagInfo info; 
    PyObject* py_name = PyDict_GetItemString(py_mask, "name"); 
    if (!py_name) { 
      PyErr_SetString(PyExc_IOError,"no 'name' in tag dict"); 
      return vec; 
    }
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) return vec; 
    PyObject* py_leaf_name = PyDict_GetItemString(py_mask, "leaf_name"); 
    if (!py_leaf_name) { 
      PyErr_SetString(PyExc_IOError,"no 'leaf_name' in tag dict"); 
      return vec; 
    }
    info.leaf_name = PyString_AsString(py_leaf_name); 
    if (PyErr_Occurred() ) return vec; 

    PyObject* py_value = PyDict_GetItemString(py_mask, "value"); 
    if (!py_value) { 
      PyErr_SetString(PyExc_IOError,"no 'value' in tag dict"); 
      return vec; 
    }
    info.value = PyLong_AsLong(py_value); 
    if (PyErr_Occurred() ) return vec; 

    vec.push_back(info); 
    
  }
  return vec; 
}

// void dummy() { 
//   FilterHist(1,0,1,new double, new int); 
// }
