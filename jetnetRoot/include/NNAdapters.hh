#ifndef NN_ADAPTERS
#define NN_ADAPTERS

class JetNet; 
class TTrainedNetwork; 

#include "TNeuralNetwork.h"

#include <string> 
#include <vector>

TNeuralNetwork* getTrainedNetwork(const JetNet&); 

void setTrainedNetwork(JetNet&, const TNeuralNetwork* trainedNetwork); 

TNeuralNetwork* getOldTrainedNetwork(std::string file_name); 
TNeuralNetwork* convertOldToNew(const TTrainedNetwork* old,
				std::vector<TNeuralNetwork::Input>); 
TTrainedNetwork* convertNewToOld(const TNeuralNetwork* new_net); 

template<class O, class I>
O convert_node(const I& in) { 
  O out; 
  out.name = in.name; 
  out.offset = in.offset; 
  out.scale = in.scale; 
  return out; 
}


#endif // NN_ADAPTERS
