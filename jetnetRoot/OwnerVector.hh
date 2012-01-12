#ifndef OWNER_VECTOR_H
#define OWNER_VECTOR_H

#include <vector>

// --- owner vector
template<typename T>
class OwnerVector : public std::vector<T>
{
public:
  OwnerVector(std::vector<T> v):  
    std::vector<T>(v)
  { 
  }
  ~OwnerVector()
  {
    for (typename std::vector<T>::iterator itr = this->begin(); 
	 itr != this->end(); 
	 itr++){
      delete *itr; 
      *itr = 0;
    }
  }
}; 

#endif // OWNER_VECTOR_H
