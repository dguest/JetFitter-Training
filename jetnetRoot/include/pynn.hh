#ifndef PY_NN_H
#define PY_NN_H

#include <Python.h>
#include <string> 
#include "pyparse.hh"

struct InputVariableInfo; 

const unsigned MAX_DOC_STRING_LENGTH = 10000; 

std::vector<InputVariableInfo> parse_input_variable_info(PyObject* py_dict); 

void build_doc(char* doc_array, 
	       std::string before, const char** input_kwds, 
	       std::string after); 


#endif 
