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
    throw ListParseException(); 
  }

  int n_strings = PyList_Size(string_list); 
  for (int i = 0; i < n_strings; i++){ 
    PyObject* the_ob = PyList_GetItem(string_list, i); 
    if (!PyString_Check(the_ob)){ 
      throw StringParseException(); 
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

std::vector<double> parse_double_list(PyObject* py_list){ 
  std::vector<double> out_vals; 
  if (py_list == 0) { 
    return out_vals; 
  }

  bool ok = PyList_Check(py_list); 
  if (!ok) {
    throw ListParseException(); 
  }

  int n_items = PyList_Size(py_list); 
  for (int i = 0; i < n_items; i++){ 
    PyObject* the_ob = PyList_GetItem(py_list, i); 
    if (!PyFloat_Check(the_ob)){ 
      throw FloatParseException(); 
    }

    float the_value = PyFloat_AsDouble(the_ob); 
    out_vals.push_back(the_value); 
  }
  return out_vals; 

}
