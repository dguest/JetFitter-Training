// python wrapper for neural net training set preperation 
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <iostream>
#include "writeNtuple_Official.hh"

extern "C" { 
  
  static const char* doc_string = 
    "inputs: TBA\n"; 

  PyObject* prep_ntuple(PyObject *self, 
			PyObject *args, 
			PyObject *keywds)
  {
    PyObject* input_file_list; 
    PyObject* observer_discriminators = PyList_New(0); 
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
    
    ok = PyList_Check(input_file_list); 
    if (!ok) { 
      PyErr_SetString(PyExc_TypeError,"first arg should be a list of files"); 
      return NULL; 
    }

    ok = PyList_Check(observer_discriminators); 
    if (!ok) { 
      PyErr_SetString(PyExc_TypeError,
		      "observer_discriminators should be a list"); 
      return NULL; 
    }

    
    std::vector<std::string> files; 
    int n_files = PyList_Size(input_file_list); 
    for (int i = 0; i < n_files; i++){ 
      PyObject* the_file = PyList_GetItem(input_file_list, i); 
      if (!PyString_Check(the_file)){ 
	PyErr_SetString(PyExc_TypeError,
			"found non-string in file list"); 
	return NULL; 
      }

      std::string the_string = PyString_AsString(the_file); 
      files.push_back(the_string); 
    }

    
    std::vector<std::string> observers; 
    int n_observers = PyList_Size(observer_discriminators); 
    for (int i = 0; i < n_observers; i++){ 
      PyObject* the_ob = PyList_GetItem(observer_discriminators, i); 
      if (!PyString_Check(the_ob)){ 
	PyErr_SetString(PyExc_TypeError,
			"found non-string in file list"); 
	return NULL; 
      }

      std::string the_string = PyString_AsString(the_ob); 
      observers.push_back(the_string); 
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

  PyMODINIT_FUNC initpyprep(void)
  {
    Py_InitModule("pyprep", keywdarg_methods);
  }

}
