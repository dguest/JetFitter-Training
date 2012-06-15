// python wrapper for neural net training set preperation 
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
#include <string> 
#include <vector>
#include <iostream>
#include <stdexcept>
#include "writeNtuple_Official.hh"
#include "writeNtuple_common.hh"
#include "writeNtuple_byPt.hh"
#include "flatNtuple.hh"
#include "pyprep.hh"
#include "pyparse.hh"
#include "ntuple_defaults.hh"
  
static char prep_ntuple_doc[MAX_DOC_STRING_LENGTH]; 

const char *prep_ntuple_kwlist[] = {
  "input_files",
  "int_variables", 
  "double_variables", 
  "observer_discriminators",
  "pt_divisions", 
  "jet_collection", 
  "jet_tagger", 
  "output_file", 
  "for_nn", 
  "randomize", 
  "debug", 
  NULL};

PyObject* prep_ntuple(PyObject *self, 
		      PyObject *args, 
		      PyObject *keywds)
{
  PyObject* input_file_list; 
  PyObject* int_variables = 0; 
  PyObject* double_variables = 0; 
  PyObject* observer_discriminators = 0; 
  PyObject* pt_divisions = 0; 
  const char* jet_collection_name = "AntiKt4TopoEMJets"; 
  const char* jet_tagger = "JetFitterCharm";
  const char* output_file_name = "reduced.root"; 
  bool for_nn = true; 
  bool randomize = false; 
  bool debug = false; 
    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, "O|OOOOssbbb", 
     // this function should take a const, and 
     // may be changed. until then we'll cast
     const_cast<char**>(prep_ntuple_kwlist),
     &input_file_list,
     &int_variables, 
     &double_variables, 
     &observer_discriminators, 
     &pt_divisions, 
     &jet_collection_name, 
     &jet_tagger, 
     &output_file_name, 
     &for_nn, 
     &randomize, 
     &debug); 

  if (!ok) return NULL;


  std::vector<std::string> files = parse_string_list(input_file_list); 
  if (PyErr_Occurred()) return 0; 
  Observers observers; 
  observers.discriminators = parse_string_list(observer_discriminators); 
  observers.int_variables = parse_string_list(int_variables); 
  observers.double_variables = parse_string_list(double_variables); 
  if (PyErr_Occurred()) return 0; 

  std::vector<double> pt_cat_vec; 
  if (pt_divisions == 0){ 
    using defopt::PT_CATEGORIES2; 
    pt_cat_vec.assign(PT_CATEGORIES2, PT_CATEGORIES2 + 6); 
  }
  else { 
    pt_cat_vec = parse_double_list(pt_divisions); 
    if (PyErr_Occurred()) return 0; 
  }

  if (debug){ 
    std::cout << "files: " << std::endl;
    for (std::vector<std::string>::const_iterator itr = files.begin(); 
	 itr != files.end(); 
	 itr++){ 
      std::cout << *itr << std::endl;
    }

    std::cout << "observers: " << std::endl;
    std::cout << observers << std::endl; 

    printf("jet collection: %s\n" , jet_collection_name);
    printf("output file: %s\n", output_file_name); 

  }

  else{ 
    try { 
      writeNtuple_Official(files, 
			   observers, 
			   pt_cat_vec, 
			   jet_collection_name, 
			   jet_tagger, 
			   output_file_name, 
			   for_nn);
    } 
    catch (const std::runtime_error& e) { 
      PyErr_SetString(PyExc_IOError,e.what()); 
      return NULL; 
    }

  }

  Py_INCREF(Py_None);

  return Py_None;
}

static const char *make_ntuple_kwlist[] = {
  "input_files",
  "int_variables", 
  "double_variables", 
  "observer_discriminators",
  "pt_divisions", 
  "jet_collection", 
  "output_dir", 
  "suffix", 
  "debug", 
  NULL};
static char make_ntuple_doc[MAX_DOC_STRING_LENGTH]; 

PyObject* make_ntuples_ptcat(PyObject *self, 
			     PyObject *args, 
			     PyObject *keywds)
{
  PyObject* input_file_list; 
  PyObject* int_variables = 0; 
  PyObject* double_variables = 0; 
  PyObject* observer_discriminators = 0; 
  PyObject* pt_divisions = 0; 
  const char* jet_collection_name = "AntiKt4TopoEMJets"; 
  const char* output_dir = "reduced"; 
  const char* suffix = "AOD"; 
  bool debug = false; 

    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, "O|OOOOsssb", 
     // this function should take a const, and 
     // may be changed. until then we'll cast
     const_cast<char**>(make_ntuple_kwlist),
     &input_file_list,
     &int_variables, 
     &double_variables, 
     &observer_discriminators, 
     &pt_divisions, 
     &jet_collection_name, 
     &output_dir, 
     &suffix, 
     &debug); 

  if (!ok) return NULL;

  std::vector<std::string> files = parse_string_list(input_file_list); 
  if (PyErr_Occurred()) return 0; 

  Observers observers; 
  observers.discriminators = parse_string_list(observer_discriminators); 
  observers.int_variables = parse_string_list(int_variables); 
  observers.double_variables = parse_string_list(double_variables); 
  if (PyErr_Occurred()) return 0; 

  std::vector<double> pt_cat_vec; 
  if (pt_divisions == 0){ 
    using defopt::PT_CATEGORIES2; 
    pt_cat_vec.assign(PT_CATEGORIES2, PT_CATEGORIES2 + 6); 
  }
  else { 
    pt_cat_vec = parse_double_list(pt_divisions); 
    if (PyErr_Occurred()) return 0; 
  }

  if (debug){ 
    std::cout << "files: " << std::endl;
    for (std::vector<std::string>::const_iterator itr = files.begin(); 
	 itr != files.end(); 
	 itr++){ 
      std::cout << *itr << std::endl;
    }

    std::cout << "observers: " << std::endl;
    std::cout << observers << std::endl; 

    printf("jet collection: %s\n" , jet_collection_name);
    printf("output dir: %s\n", output_dir); 

  }

  else{ 
  
    try { 
      writeNtuple_byPt(files, 
		       observers, 
		       pt_cat_vec, 
		       jet_collection_name, 
		       output_dir, 
		       suffix); 
    }
    catch (LoadOfficialDSException e) { 
      PyErr_SetString(PyExc_IOError, "something went horribly wrong "
		      "while trying to load datasets"); 
      return NULL; 
    }
  }

  Py_INCREF(Py_None);

  return Py_None;
}

static const char *flat_ntuple_kwlist[] = {
  "input_files",
  "int_variables", 
  "double_variables", 
  "observer_discriminators",
  "pt_divisions", 
  "jet_collection", 
  "jet_tagger", 
  "output_file", 
  "debug", 
  NULL};
static char flat_ntuple_doc[MAX_DOC_STRING_LENGTH]; 

PyObject* make_flat_ntuple(PyObject *self, 
			   PyObject *args, 
			   PyObject *keywds)
{
  PyObject* input_file_list; 
  PyObject* int_variables = 0; 
  PyObject* double_variables = 0; 
  PyObject* observer_discriminators = 0; 
  PyObject* pt_divisions = 0; 
  const char* jet_collection_name = "AntiKt4TopoEMJetsReTagged"; 
  const char* jet_tagger = "JetFitterCharm";
  const char* output_file_name = "nothing.root"; 
  bool debug = false; 

    
  bool ok = PyArg_ParseTupleAndKeywords
    (args, keywds, "O|OOOOsssb", 
     // this function should take a const, and 
     // may be changed. until then we'll cast
     const_cast<char**>(flat_ntuple_kwlist),
     &input_file_list,
     &int_variables, 
     &double_variables, 
     &observer_discriminators, 
     &pt_divisions, 
     &jet_collection_name, 
     &jet_tagger, 
     &output_file_name, 
     &debug); 

  if (!ok) return NULL;

  std::vector<std::string> files = parse_string_list(input_file_list); 
  if (PyErr_Occurred()) return 0; 

  Observers observers; 
  observers.discriminators = parse_string_list(observer_discriminators); 
  observers.int_variables = parse_string_list(int_variables); 
  observers.double_variables = parse_string_list(double_variables); 
  if (PyErr_Occurred()) return 0; 

  std::vector<double> pt_cat_vec = parse_double_list(pt_divisions); 
  if (PyErr_Occurred()) return 0; 

  

  if (debug){ 
    std::cout << "files: " << std::endl;
    for (std::vector<std::string>::const_iterator itr = files.begin(); 
	 itr != files.end(); 
	 itr++){ 
      std::cout << *itr << std::endl;
    }

    std::cout << "observers: " << std::endl;
    std::cout << observers << std::endl; 

    printf("jet collection: %s\n" , jet_collection_name);
    printf("output file: %s\n", output_file_name); 

  }

  else{ 
  
    try { 
      flatNtuple(files, 
		 observers, 
		 pt_cat_vec, 
		 jet_collection_name, 
		 jet_tagger, 
		 output_file_name); 
    }
    catch (const std::runtime_error& e) { 
      PyErr_SetString(PyExc_IOError,e.what()); 
      return NULL; 
    }
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
   prep_ntuple_doc},
  {"make_ntuples_ptcat", (PyCFunction)make_ntuples_ptcat, 
   METH_VARARGS | METH_KEYWORDS,
   make_ntuple_doc},
  {"make_flat_ntuple", (PyCFunction)make_flat_ntuple, 
   METH_VARARGS | METH_KEYWORDS,
   flat_ntuple_doc},
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 


  PyMODINIT_FUNC initpyprep(void)
  {
    build_doc(prep_ntuple_doc, prep_ntuple_kwlist); 
    build_doc(make_ntuple_doc, make_ntuple_kwlist); 
    build_doc(flat_ntuple_doc, flat_ntuple_kwlist); 
    
    Py_InitModule("pyprep", keywdarg_methods);
  }

}

void build_doc(char* doc_array, const char** input_kwds){ 
  for (int n = 0; n < 20; n++) { 
    const char* this_str = input_kwds[n]; 
    if (! this_str) break; 
    strcat(doc_array, this_str); 
    strcat(doc_array,"\n"); 
    assert(strlen(doc_array) < MAX_DOC_STRING_LENGTH); 
  }
}
