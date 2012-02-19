#ifndef PY_NN_H
#define PY_NN_H

#include <Python.h>
#include "pyparse.hh"

struct InputVariableInfo; 

std::vector<InputVariableInfo> parse_input_variable_info(PyObject* py_dict); 

#endif 
