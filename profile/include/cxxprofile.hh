#ifndef PY_PREP_H
#define PY_PREP_H

#include <Python.h>
#include <vector>
#include <string> 
#include <utility> // pair 
class LeafInfo; 
class LeafInfoPairs; 

std::vector<LeafInfo> build_leaf_info_vec(PyObject*); 
LeafInfoPairs build_plot2d_vec(PyObject*); 
LeafInfo build_leaf_info(PyObject*); 
std::vector<std::string> build_string_vec(PyObject*); 
std::vector<double> build_double_vec(PyObject*); 


#endif 
