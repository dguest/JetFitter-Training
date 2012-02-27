#ifndef NORMED_INPUT_H
#define NORMED_INPUT_H

#include <string> 
#include <TTree.h>
#include <ostream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "nnExceptions.hh"

namespace norm { 
  const std::string info_tree_name = "normalization_info"; 
}


// --- data structure for input info
struct InputVariableInfo { 
  std::string name; 
  float offset; 
  float scale; 
}; 



class NormedInputBase
{
public: 
  virtual int set_tree(TTree* ) {}; 
  virtual float get_normed() const {}; 
  virtual float get_offset() const {}; 
  virtual float get_scale() const {}; 
  virtual std::string get_name() const {}; 
  virtual int add_passthrough_branch_to(TTree*, std::string = "") const {}; 
  friend std::ostream& operator<<(std::ostream&, const NormedInputBase&);
private: 
  virtual void print_to(std::ostream&) const = 0; 
}; 


template<typename T>
class NormedInput : public NormedInputBase
{
public: 
  // -- setup
  NormedInput(const InputVariableInfo& info,TTree* = 0); 
  // NormedInput(const std::string& name, TTree* = 0); 
  NormedInput(const std::string& name, 
	      float offset, float scale, TTree* = 0); 
  ~NormedInput(); 
  int set_tree(TTree*); 

  // --getters 
  float get_normed() const; 
  float get_offset() const { return _info.offset; }
  float get_scale() const { return _info.scale; }  
  std::string get_name() const { return _info.name; }
  T* get() const { return _buffer; } 

  // --- output 
  int add_passthrough_branch_to(TTree* output_tree, 
				std::string branch_name = "") const; 

  template <typename Q>
  friend std::ostream& operator<<(std::ostream&, const NormedInput<Q>&); 
private: 
  T* _buffer; 
  InputVariableInfo _info; 
  void print_to(std::ostream& out) const; 
}; 


// ---- container 

class InputVariableContainer : public boost::ptr_vector<NormedInputBase>
{
public: 
  int write_to_file(TFile*, 
		    std::string tree_name = norm::info_tree_name) const; 

  // builds from info_tree, if an entry in info_tree is not found, 
  int build_from_tree(TTree* info_tree, TTree* reduced_dataset); 

  // throws a MissingLeafException if variable not found
  int add_variable(const InputVariableInfo& variable, 
		   TTree* reduced_dataset); 
  int add_variable(const std::string& name, TTree* reduced_dataset); 

  int set_hardcoded_defaults(TTree* reduced_dataset); 

}; 

// --- exceptions 
class LoadNormalizationException: public NormalizationException {}; 
class MissingLeafException: public NormalizationException 
{
public: 
  MissingLeafException(std::string leaf_name, std::string chain_name); 
  std::string leaf_name() const; 
  std::string chain_name() const; 
private: 
  std::string _chain_name; 
  std::string _leaf_name; 
}; 



//=======================================================
//========= template implementation =====================
//=======================================================

template<typename T>
NormedInput<T>::NormedInput(const InputVariableInfo& info, TTree* tree): 
  _info(info)
{ 
  _buffer = new T; 
  if (tree){ 
    this->set_tree(tree); 
  }
}

// template<typename T>
// NormedInput<T>::NormedInput(const std::string& name, TTree* tree)
// {
//   _info.name = name; 
//   _info.offset = 0; 
//   _info.scale = 0; 
//   if (tree){ 
//     this->set_tree(tree); 
//   }
// }

template<typename T>
NormedInput<T>::NormedInput(const std::string& name, 
			    float offset, float scale, 
			    TTree* tree)
{ 
  _info.name = name; 
  _info.offset = offset; 
  _info.scale = scale; 
  _buffer = new T; 
  if (tree){ 
    this->set_tree(tree); 
  }
}


template<typename T>
int NormedInput<T>::set_tree(TTree* input_tree){ 
  input_tree->SetBranchStatus(_info.name.c_str(),1); 
  input_tree->SetBranchAddress(_info.name.c_str(), _buffer); 
  return 0; 
}

template<typename T> 
NormedInput<T>::~NormedInput(){ 
  delete _buffer; 
  _buffer = 0; 
}

template<typename T>
float NormedInput<T>::get_normed() const 
{
  float output = (float(*_buffer) + _info.offset) * _info.scale; 
  return output; 
}

template<typename T>
int NormedInput<T>::add_passthrough_branch_to(TTree* output_tree, 
					      std::string branch_name) const
{
  if (branch_name.size() == 0){ 
    branch_name = _info.name; 
  }
  output_tree->Branch(branch_name.c_str(), _buffer); 
  return 0; 
}

template<typename T>
void NormedInput<T>::print_to(std::ostream& out) const
{
  out << "input \"" << _info.name << "\" -- offset: " << _info.offset << 
    " scale: " << _info.scale; 
}

template <typename Q>
std::ostream& operator<<(std::ostream& out, const NormedInput<Q>& n)
{ 
  n.print_to(out); 
  return out; 
}; 




#endif // NORMED_INPUT_H
