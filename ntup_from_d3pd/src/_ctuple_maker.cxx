// python wrapper for ctuple builder
// Author: Daniel Guest (dguest@cern.ch)
#include <Python.h>
// #include <string> 
// #include <vector>
// #include <map>
#include <stdexcept>

#include "JetFactory.hh"

static unsigned parse_flags(const char* flags);


static PyObject* py_cutflow(PyObject *self, 
			    PyObject *args)
{
  PyObject* py_input_files; 
  const char* flags_str = ""; 
  const char* out_ntuple = ""; 

  RunInfo info; 
  bool ok = PyArg_ParseTuple
    (args,"Oi|ss:cutflow", &py_input_files, &info.run_number, 
     &flags_str, &out_ntuple); 
  if (!ok) return NULL;

  int n_files = PyList_Size(py_input_files); 
  if (PyErr_Occurred()) return NULL; 
    
  std::vector<std::string> input_files; 
  for (int n = 0; n < n_files; n++) { 
    PyObject* py_file_name = PyList_GetItem(py_input_files, n); 
    std::string file_name = PyString_AsString(py_file_name); 
    if (PyErr_Occurred()) return NULL; 
    input_files.push_back(file_name);     
  }
  
  unsigned flags = 0; 
  if (strchr(flags_str,'v')) flags |= cutflag::verbose; 
  if (strchr(flags_str,'d')) flags |= cutflag::is_data; 
  if (strchr(flags_str,'s')) flags |= cutflag::is_signal; 
  if (strchr(flags_str,'p')) flags |= cutflag::use_low_pt_jets; 
  if (strchr(flags_str,'d')) flags |= cutflag::debug_susy; 
  if (strchr(flags_str,'r')) flags |= cutflag::save_ratios; 
  if (strchr(flags_str,'w')) flags |= cutflag::save_flavor_wt; 
  if (strchr(flags_str,'f')) flags |= cutflag::is_atlfast; 
  if (strchr(flags_str,'t')) flags |= cutflag::save_truth; 
  if (strchr(flags_str,'c')) flags |= cutflag::jetfitter_charm; 
  if (strchr(flags_str,'m')) flags |= cutflag::mv3; 

  // other taggers not implemented yet
  assert( (flags & (cutflag::jetfitter_charm | cutflag::mv3)) == 0); 

  typedef std::vector<std::pair<std::string, int> > CCOut; 
  CCOut pass_numbers; 
  try { 
    pass_numbers = run_cutflow(input_files, info, flags, out_ntuple); 
  }
  catch (std::runtime_error e) { 
    PyErr_SetString(PyExc_RuntimeError, e.what()); 
    return NULL; 
  }

  PyObject* out_list = PyList_New(0); 
  for (CCOut::const_iterator itr = pass_numbers.begin(); 
       itr != pass_numbers.end(); 
       itr++){ 
    PyObject* tuple = Py_BuildValue("si", itr->first.c_str(), itr->second);
    if(PyList_Append(out_list,tuple)) { 
      return NULL; 
    }
  }
  
  return out_list; 

}


template<typename T>
static PyObject* py_analysis_alg(PyObject *self, PyObject *args)
{
  const char* input_file = ""; 
  PyObject* bits = 0; 
  const char* output_file = ""; 
  const char* flags = ""; 
  PyObject* floats_dict = 0; 

  bool ok = PyArg_ParseTuple
    (args,"sOs|sO:algo", 
     &input_file, &bits, &output_file, &flags, &floats_dict); 
  if (!ok) return NULL;

  unsigned bitflags = parse_flags(flags); 

  int ret_val = 0; 
  try { 
    T builder(input_file, bitflags); 

    const int n_bits = PyList_Size(bits); 
    if (PyErr_Occurred()) return NULL; 
    for (int bit_n = 0; bit_n < n_bits; bit_n++) { 
      PyObject* entry = PyList_GetItem(bits, bit_n); 
      const char* name = ""; 
      unsigned mask = 0; 
      ok = PyArg_ParseTuple(entry, "sk:parsemask", &name, &mask); 
      if (!ok) return NULL; 
      builder.add_cut_mask(name, mask); 
    }
    PyObject* dic_key = 0; 
    PyObject* dic_val = 0; 
    int pos = 0; 
    if (floats_dict) { 
      while (PyDict_Next(floats_dict, &pos, &dic_key, &dic_val)) { 
	char* key_val = PyString_AsString(dic_key); 
	if (PyErr_Occurred()) return NULL; 
	float val_val = PyFloat_AsDouble(dic_val); 
	if (PyErr_Occurred()) return NULL; 
	builder.set_float(key_val, val_val); 
      }
    }
    
    ret_val = builder.build(); 
    builder.save(output_file); 
  }
  catch (std::runtime_error e) { 
    PyErr_SetString(PyExc_RuntimeError, e.what()); 
    return NULL; 
  }

  PyObject* tuple = Py_BuildValue("i", ret_val);
  
  return tuple; 

}

static PyMethodDef methods[] = {
  {"_stacksusy", py_analysis_alg<HistBuilder>, METH_VARARGS, 
   "don't ask, read the source"},
  {"_hypersusy", py_analysis_alg<HyperBuilder>, METH_VARARGS, 
   "eat a failure sandwich"}, 
  {"_cutflow", py_analysis_alg<CutflowBuilder>, METH_VARARGS, 
   "eat a failure sandwich"}, 
  {NULL, NULL, 0, NULL}   /* sentinel */
};

extern "C" { 

  PyMODINIT_FUNC init_hyperstack(void)
  {
    Py_InitModule("_hyperstack", methods);
  }

}


static unsigned parse_flags(const char* flags){ 
  using namespace buildflag; 
  unsigned bitflags = 0; 
  if(strchr(flags,'v')) bitflags |= verbose; 
  if(strchr(flags,'t')) bitflags |= fill_truth; 
  if(strchr(flags,'i')) bitflags |= leading_jet_btag; 
  if(strchr(flags,'m')) bitflags |= mttop; 
  if(strchr(flags,'b')) bitflags |= disable_c_tags; 
  return bitflags; 
}
