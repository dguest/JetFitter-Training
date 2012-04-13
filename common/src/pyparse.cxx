#include <Python.h>
#include <vector>
#include <string>

#include "pyparse.hh"


std::vector<std::string> parse_string_list(PyObject* string_list){ 
  std::vector<std::string> strings; 
  if (string_list == 0) { 
    return strings; 
  }

  bool ok = PyList_Check(string_list); 
  if (!ok) {
    PyErr_SetString(PyExc_TypeError,"expected a list, found a not-list"); 
    return strings;
  }

  int n_strings = PyList_Size(string_list); 
  for (int i = 0; i < n_strings; i++){ 
    PyObject* the_ob = PyList_GetItem(string_list, i); 
    if (!PyString_Check(the_ob)){ 
      PyErr_SetString(PyExc_TypeError,"expected a string in a list"); 
      return strings; 
    }

    std::string the_string = PyString_AsString(the_ob); 
    strings.push_back(the_string); 
  }
  return strings; 

}




std::vector<int> parse_int_tuple(PyObject* py_list){ 
  std::vector<int> ints; 
  if (py_list == 0) { 
    return ints; 
  }

  bool ok = PyTuple_Check(py_list); 
  if (!ok) {
    PyErr_SetString(PyExc_TypeError,"expected a tuple of ints"); 
    return ints; 
  }

  int n_items = PyTuple_Size(py_list); 
  for (int i = 0; i < n_items; i++){ 
    PyObject* the_ob = PyTuple_GetItem(py_list, i); 
    if (!PyInt_Check(the_ob)){ 
      PyErr_SetString(PyExc_TypeError,"expected an int in this tuple"); 
      return ints; 
    }

    int the_int = PyInt_AsLong(the_ob); 
    ints.push_back(the_int); 
  }
  return ints; 

}

std::vector<double> parse_double_list(PyObject* py_list){ 
  std::vector<double> out_vals; 
  if (py_list == 0) { 
    return out_vals; 
  }

  bool ok = PyList_Check(py_list); 
  if (!ok) {
    PyErr_SetString(PyExc_TypeError,"expected a list of floats"); 
    return out_vals; 
  }

  int n_items = PyList_Size(py_list); 
  for (int i = 0; i < n_items; i++){ 
    PyObject* the_ob = PyList_GetItem(py_list, i); 
    if (!PyFloat_Check(the_ob)){ 
      PyErr_SetString(PyExc_TypeError,"expected a list of floats"); 
      return out_vals; 
    }

    float the_value = PyFloat_AsDouble(the_ob); 
    out_vals.push_back(the_value); 
  }
  return out_vals; 

}

std::map<std::string,double> parse_double_dict(PyObject* in_dict)
{
  std::map<std::string, double> double_map; 
  if (in_dict == 0){ 
    return double_map; 
  }

  bool ok = PyDict_Check(in_dict); 
  if (!ok) { 
    PyErr_SetString(PyExc_TypeError,"expected a dict, "
		    "key = varname, value = (offset, scale)"); 
    return double_map; 
  }

  PyObject* key = 0; 
  PyObject* value = 0; 
  Py_ssize_t pos = 0; 
  while (PyDict_Next(in_dict, &pos, &key, &value) ) { 
    bool ok_key = PyString_Check(key); 
    bool ok_float = PyFloat_Check(value); 
    if (!ok_key || !ok_float ) {
      PyErr_SetString(PyExc_TypeError,"expected a dict, "
		      "key = varname, value = (offset, scale)"); 
      return double_map; 
    }

    std::string the_key = PyString_AsString(key); 
    double_map[the_key] = PyFloat_AsDouble(value); 
  }
  return double_map; 

}
