#ifndef PY_PREP_H
#define PY_PREP_H

#include <Python.h>
#include <vector>
#include <string> 
class LeafInfo; 


std::vector<LeafInfo> build_leaf_info(PyObject*); 
std::vector<std::string> build_string_vec(PyObject*); 


#endif 
