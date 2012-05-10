#ifndef NN_EXCEPTIONS_H
#define NN_EXCEPTIONS_H

#include <string> 

class NNException {}; 

class LoadReducedDSException: public NNException 
{
public: 
  LoadReducedDSException(const char* file_name = "unknown", 
			 const char* tree_name = "unknown"); 
  virtual ~LoadReducedDSException() throw() {}; 
  virtual const char* what() const throw(); 
  std::string info() const; 
private: 
  const char* _file_name; 
  const char* _tree_name; 
}; 

class NormalizationException: public NNException {};

#endif // NN_EXCEPTIONS_H
