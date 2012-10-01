// python functions to prune trees
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <set> 
#include <stdexcept>
// #include <algorithm> // sort

#include "prune_tree.hh"
#include "cxxprune.hh"
  
static const std::string simple_name = "simple_prune"; 

static const std::string simple_additonal = 
  "Builds file 'out_file', cuts on all *_cuts, and only saves subset\n"
  "flags:\n"
  "\tv -- verbose\n"; 

static const char* simple_kwlist[] = {
  "in_file",
  "out_file", 
  "tree", 
  "int_cuts", 
  "double_cuts", 
  "mask_cuts", 
  "subset", 
  "max_entries",
  "flags", 
  NULL};

static char simple_doc[MAX_DOC_STRING_LENGTH]; 

PyObject* py_simple_prune(PyObject *self, 
			  PyObject *args, 
			  PyObject *keywds)
{
  const char* file_name; 
  const char* tree_name = "SVTree"; 
  const char* output_file; 
  PyObject* py_int_cuts = 0; 
  PyObject* py_double_cuts = 0; 
  PyObject* py_mask_cuts = 0; 
  PyObject* py_subset = 0; 
  int max_entries = -1; 
  const char* flags; 
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, 
     "ss|sOOOOis:simple_prune", 
     // I think python people argue about whether this should be 
     // a const char** or a char**
     const_cast<char**>(simple_kwlist),
     &file_name,
     &output_file, 
     &tree_name, 
     &py_int_cuts, 
     &py_double_cuts, 
     &py_mask_cuts, 
     &py_subset, 
     &max_entries, 
     &flags); 

  if (!ok) return NULL;

  unsigned options = 0; 

  if (strchr(flags,'v')) options |= opt::verbose; 

  Cuts cuts; 
  int n_int_cuts = 0; 
  if (py_int_cuts)
    n_int_cuts = PyList_Size(py_int_cuts); 

  if (PyErr_Occurred()) { 
    return NULL; 
  }
  for (int cut_n = 0; cut_n < n_int_cuts; cut_n++) { 
    SubTreeIntInfo info; 
    PyObject* py_cut = PyList_GetItem(py_int_cuts, cut_n); 
    if (PySequence_Size(py_cut) != 2) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect a list of tuples (name, value) for int_cuts"); 
      return NULL; 
    }
    PyObject* py_name = PySequence_GetItem(py_cut, 0); 
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect tuples (name, value) for int_cuts, name"
		      " must be a string"); 
      return NULL; 
    }
    PyObject* py_value = PySequence_GetItem(py_cut, 1); 
    info.value = PyInt_AsLong(py_value); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect tuples (name, value) for int_cuts, value"
		      " must be an int"); 
      return NULL; 
    }
    cuts.ints.push_back(info); 
  }

  int n_double_cuts = 0; 
  if (py_double_cuts)
    n_double_cuts = PyList_Size(py_double_cuts); 
  if (PyErr_Occurred()) { 
    return NULL; 
  }
  for (int cut_n = 0; cut_n < n_double_cuts; cut_n++) { 
    SubTreeDoubleInfo info; 
    PyObject* py_cut = PyList_GetItem(py_double_cuts, cut_n); 
    if (PySequence_Size(py_cut) != 3) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect a list of tuples (name, low, high)"
		      " for double_cuts"); 
      return NULL; 
    }
    PyObject* py_name = PySequence_GetItem(py_cut, 0); 
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect tuples (name, low, high) for double_cuts, name"
		      " must be a string"); 
      return NULL; 
    }
    PyObject* py_low = PySequence_GetItem(py_cut, 1); 
    info.low = PyFloat_AsDouble(py_low); 
    PyObject* py_high = PySequence_GetItem(py_cut, 2); 
    info.high = PyFloat_AsDouble(py_high); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect tuples (name, low, high) for double_cuts"
		      " high and low must be floats"); 
      return NULL; 
    }
    cuts.doubles.push_back(info); 
  }



  int n_mask_cuts = 0; 
  if (py_mask_cuts)
    n_mask_cuts = PyList_Size(py_mask_cuts); 

  if (PyErr_Occurred()) { 
    return NULL; 
  }
  for (int cut_n = 0; cut_n < n_mask_cuts; cut_n++) { 
    SubTreeUnsignedInfo info; 
    PyObject* py_cut = PyList_GetItem(py_mask_cuts, cut_n); 
    if (PySequence_Size(py_cut) != 3) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect a list of tuples (name, value) for int_cuts"); 
      return NULL; 
    }
    PyObject* py_name = PySequence_GetItem(py_cut, 0); 
    info.name = PyString_AsString(py_name); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect tuples (name, value) for int_cuts, name"
		      " must be a string"); 
      return NULL; 
    }
    PyObject* py_req = PySequence_GetItem(py_cut, 1); 
    info.required = PyInt_AsUnsignedLongMask(py_req); 
    PyObject* py_veto = PySequence_GetItem(py_cut, 2); 
    info.veto = PyInt_AsUnsignedLongMask(py_veto); 
    if (PyErr_Occurred() ) { 
      PyErr_SetString(PyExc_TypeError,
		      "expect tuples (name, required, veto) for mask_cuts"
		      ", values must be unsigned ints"); 
      return NULL; 
    }
    cuts.bits.push_back(info); 
  }




  std::set<std::string> subset; 
  if (py_subset) { 
    PyObject* iterator = PyObject_GetIter(py_subset); 
    if (!iterator) return NULL; 

    PyObject* item; 
    while ( (item = PyIter_Next(iterator)) ) { 
      
      std::string var = PyString_AsString(item); 
      if (PyErr_Occurred()) { 
	PyErr_SetString(PyExc_TypeError, "subset should be strings"); 
	return NULL; 
      }
      subset.insert(var); 

      Py_DECREF(item); 
    }
    Py_DECREF(iterator); 
  }

  int return_code; 
  try { 
    return_code = simple_prune(file_name, tree_name, 
			       cuts, 
			       subset, 
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
  {simple_name.c_str(), (PyCFunction)py_simple_prune, 
   METH_VARARGS | METH_KEYWORDS,
   simple_doc},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 

  PyMODINIT_FUNC initcxxprune(void)
  {
    build_doc(simple_doc, 
	      simple_name + "(", simple_kwlist, ")\n" + simple_additonal); 
    Py_InitModule("cxxprune", keywdarg_methods);
  }

}

void build_doc(char* doc_array, 
	       std::string b, const char** input_kwds, std::string a){ 
  strcat(doc_array, b.c_str()); 
  for (int n = 0; n < 20; n++) { 
    const char* this_str = input_kwds[n]; 
    if (! this_str) break; 
    if (n != 0) strcat(doc_array,", "); 
    strcat(doc_array, this_str); 
  }
  strcat(doc_array, a.c_str()); 
  assert(strlen(doc_array) < MAX_DOC_STRING_LENGTH); 
}

