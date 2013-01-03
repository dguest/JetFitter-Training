// python wrapper for ctuple builder
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
// #include <string> 
// #include <vector>
// #include <map>
#include <stdexcept>

#include "TreeTranslator.hh"

static std::vector<std::string> inpt_vec_from_list(PyObject* list); 
static void feed_dict(TreeTranslator&, PyObject*); 
static unsigned parse_flags(const char*); 

static PyObject* ntup_from_d3pd(PyObject *self, 
				PyObject *args)
{
  PyObject* py_input_files; 
  const char* input_tree = ""; 
  const char* jet_collection = ""; 
  const char* output_file = ""; 
  PyObject* floats_dict = 0; 
  const char* flags = "";

  bool ok = PyArg_ParseTuple
    (args,"Osss|Os:ctuple_maker", &py_input_files, &input_tree, 
     &jet_collection, &output_file, &floats_dict, &flags); 
  if (!ok) return NULL;

  std::vector<std::string> input_files = inpt_vec_from_list(py_input_files); 
  if (PyErr_Occurred()) return NULL; 

  unsigned bits = parse_flags(flags); 

  try { 
    TreeTranslator translator(input_tree, output_file, "SVTree", bits); 
    translator.add_collection(jet_collection); 
    feed_dict(translator, floats_dict); 
    if (PyErr_Occurred()) return NULL; 
    translator.translate(input_files); 
  }
  catch (std::runtime_error e) { 
    PyErr_SetString(PyExc_RuntimeError, e.what()); 
    return NULL; 
  }

  return Py_BuildValue(""); 
}



static PyMethodDef methods[] = {
  {"_ntup_from_d3pd", ntup_from_d3pd, METH_VARARGS, 
   "don't ask, read the source"},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 

  PyMODINIT_FUNC init_ctuple_maker(void)
  {
    Py_InitModule("_ctuple_maker", methods);
  }

}

static std::vector<std::string> inpt_vec_from_list(PyObject* list){ 
  std::vector<std::string> input_files; 
  int n_files = PyList_Size(list); 

  if (PyErr_Occurred()) return input_files; 

  for (int n = 0; n < n_files; n++) { 
    PyObject* py_file_name = PyList_GetItem(list, n); 
    std::string file_name = PyString_AsString(py_file_name); 
    if (PyErr_Occurred()) return input_files; 
    input_files.push_back(file_name);     
  }
  return input_files; 
}

static void feed_dict(TreeTranslator& translator, PyObject* dict) { 
  if (dict) { 
    PyObject* py_key; 
    PyObject* py_value; 
    int pos = 0; 
    while (PyDict_Next(dict, &pos, &py_key, &py_value)) { 
      std::string key = PyString_AsString(py_key); 
      double value = PyFloat_AsDouble(py_value); 
      if (PyErr_Occurred()) return; 
      translator.set_float(key, value); 
    }
  }
}

static unsigned parse_flags(const char* str) { 
  using namespace trans; 
  unsigned flags = 0; 
  if (strchr(str, 'u')) flags |= skip_taus; 
  return flags; 
}
