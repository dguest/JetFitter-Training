#ifndef NN_ADAPTERS
#define NN_ADAPTERS

class JetNet; 
class TTrainedNetwork; 

#include "TFlavorNetwork.h"

#include <string> 
#include <vector>

TFlavorNetwork* getTrainedNetwork(const JetNet&); 

void setTrainedNetwork(JetNet&, const TFlavorNetwork* trainedNetwork); 

TFlavorNetwork* getOldTrainedNetwork(std::string file_name); 
TFlavorNetwork* convertOldToNew(const TTrainedNetwork* old,
				std::vector<TFlavorNetwork::Input>); 
TTrainedNetwork* convertNewToOld(const TFlavorNetwork* new_net); 

template<class O, class I>
O convert_node(const I& in) { 
  O out; 
  out.name = in.name; 
  out.offset = in.offset; 
  out.scale = in.scale; 
  return out; 
}


#endif // NN_ADAPTERS
