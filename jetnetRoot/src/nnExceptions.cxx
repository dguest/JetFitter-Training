#include "nnExceptions.hh"
#include <sstream> 


LoadReducedDSException::LoadReducedDSException(const char* file, 
					       const char* tree): 
  _file_name(file), 
  _tree_name(tree)
{
}

const char* LoadReducedDSException::what() const throw()
{
  return _file_name; 
}
std::string LoadReducedDSException::info() const 
{
  std::stringstream out; 
  out << "file: " << _file_name << " tree: " << _tree_name; 
  return out.str(); 
}

