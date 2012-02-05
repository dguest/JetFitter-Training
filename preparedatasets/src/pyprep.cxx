// python wrapper for neural net training set preperation 
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <iostream>
#include "writeNtuple_Official.hh"
#include "pyprep.hh"

  
static const char* doc_string = 
  "inputs: TBA\n"; 

PyObject* prep_ntuple(PyObject *self, 
		      PyObject *args, 
		      PyObject *keywds)
{
  PyObject* input_file_list; 
  PyObject* observer_discriminators = 0; 
  const char* jet_collection_name = "AntiKt4TopoEMJets"; 
  const char* output_file_name = "reduced.root"; 
  bool debug = false; 

  const char *kwlist[] = {
    "input_file_list",
    "observer_discriminators", 
    "jet_collection_name", 
    "output_file_name", 
    "debug", 
    NULL};
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, "O|Ossb", 
     // this function should take a const, and 
     // may be changed. until then we'll cast
     const_cast<char**>(kwlist),
     &input_file_list,
     &observer_discriminators, 
     &jet_collection_name, 
     &output_file_name, 
     &debug); 

  if (!ok) return NULL;

  std::vector<std::string> files; 
  std::vector<std::string> observers; 
  try { 
    files = parse_string_list(input_file_list); 
    observers = parse_string_list(observer_discriminators); 
  }
  catch(ParseException e) { 
    PyErr_SetString(PyExc_TypeError,
		    "expected a list of strings, found something else"); 
    return NULL; 
  }

  if (debug){ 
    std::cout << "files: " << std::endl;
    for (std::vector<std::string>::const_iterator itr = files.begin(); 
	 itr != files.end(); 
	 itr++){ 
      std::cout << *itr << std::endl;
    }

    std::cout << "observers: " << std::endl;
    for (std::vector<std::string>::const_iterator itr = observers.begin(); 
	 itr != observers.end(); 
	 itr++){ 
      std::cout << *itr << std::endl;
    }
      
    printf("jet collection: %s\n" , jet_collection_name);
    printf("output file: %s\n", output_file_name); 

  }

  else{ 

    writeNtuple_Official(files, observers, 
			 jet_collection_name, 
			 output_file_name); 

  }

  Py_INCREF(Py_None);

  return Py_None;
}


static PyMethodDef keywdarg_methods[] = {
  // The cast of the function is necessary since PyCFunction values
  // only take two PyObject* parameters, and keywdarg() takes
  // three.
  {"prep_ntuple", (PyCFunction)prep_ntuple, 
   METH_VARARGS | METH_KEYWORDS,
   doc_string},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 


  PyMODINIT_FUNC initpyprep(void)
  {
    Py_InitModule("pyprep", keywdarg_methods);
  }

}

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
