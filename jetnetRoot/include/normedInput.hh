#ifndef NORMED_INPUT_H
#define NORMED_INPUT_H

#include <string> 
#include <TTree.h>
#include <ostream>

class NormedInputBase
{
public: 
  virtual ~NormedInputBase() = 0; 
  virtual int set_tree(TTree* ); 
  virtual float get_normed(); 
}; 

template<typename T>
class NormedInput : public NormedInputBase
{
public: 
  NormedInput(std::string name, 
	      TTree* input_tree, 
	      float offset, 
	      float scale); 
  ~NormedInput(); 
  int set_tree(TTree*); 
  float get_normed() const; 
  T get() const; 
  template <typename Q>
  friend std::ostream& operator<<(std::ostream&, const NormedInput<Q>&); 
private: 
  T* _buffer; 
  float _offset; 
  float _scale; 
  std::string _name; 
}; 

template<typename T>
NormedInput<T>::NormedInput(std::string name, TTree* input_tree, 
			    float offset, float scale): 
  _offset(offset), 
  _scale(scale), 
  _name(name)
{ 
  _buffer = new T; 
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
  float output = (float(_buffer) + _offset)* _scale; 
  return output; 
}

template<typename T>
T NormedInput<T>::get() const 
{
  return *_buffer; 
}

template <typename Q>
std::ostream& operator<<(std::ostream& out, const NormedInput<Q>& n)
{ 
  out << "input \"" << n._name << "\"-- offset: " << n._offset << 
    " scale: " << n._scale; 
  return out; 
}; 



#endif // NORMED_INPUT_H
