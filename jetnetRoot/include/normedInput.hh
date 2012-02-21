#ifndef NORMED_INPUT_H
#define NORMED_INPUT_H

#include <string> 
#include <TTree.h>
#include <ostream>
#include <boost/ptr_container/ptr_vector.hpp>
#include "nnExceptions.hh"


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


class NormedInputBase
{
public: 
  virtual ~NormedInputBase() = 0; 
  virtual int set_tree(TTree* ) {}; 
  virtual float get_normed() const {}; 
  virtual float get_offset() const {}; 
  virtual float get_scale() const {}; 
  virtual std::string get_name() const {}; 
}; 


template<typename T>
class NormedInput : public NormedInputBase
{
public: 
  NormedInput(std::string name, 
	      float offset, 
	      float scale); 
  NormedInput(std::string name, 
	      float offset, 
	      float scale, 
	      TTree*); 
  ~NormedInput(); 
  int set_tree(TTree*); 
  float get_normed() const; 
  float get_offset() const { return _offset; }
  float get_scale() const { return _scale; }  
  std::string get_name() const { return _name; }
  T get() const { return _buffer; } 
  template <typename Q>
  friend std::ostream& operator<<(std::ostream&, const NormedInput<Q>&); 
private: 
  T* _buffer; 
  float _offset; 
  float _scale; 
  std::string _name; 
}; 


// ----------- persistence helpers -----------------



class InputVariableContainer : public boost::ptr_vector<NormedInputBase>
{
public: 
  int write_to_file(TFile*, 
		    std::string tree_name = "normalization_info") const; 

  int build_from_tree(TTree* info_tree, TTree* reduced_dataset); 
}; 

//=======================================================
//========= template implementation =====================
//=======================================================


template<typename T>
NormedInput<T>::NormedInput(std::string name, 
			    float offset, float scale): 
  _offset(offset), 
  _scale(scale), 
  _name(name)
{ 
  _buffer = new T; 
}

template<typename T>
NormedInput<T>::NormedInput(std::string name, 
			    float offset, float scale, 
			    TTree* tree): 
  _offset(offset), 
  _scale(scale), 
  _name(name)
{ 
  _buffer = new T; 
  this->set_tree(tree); 
}


template<typename T>
int NormedInput<T>::set_tree(TTree* input_tree){ 
  input_tree->SetBranchStatus(_name.c_str(),1); 
  input_tree->SetBranchAddress(_name.c_str(), _buffer); 
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
  float output = (float(*_buffer) + _offset) * _scale; 
  return output; 
}


template <typename Q>
std::ostream& operator<<(std::ostream& out, const NormedInput<Q>& n)
{ 
  out << "input \"" << n._name << "\"-- offset: " << n._offset << 
    " scale: " << n._scale; 
  return out; 
}; 




#endif // NORMED_INPUT_H
