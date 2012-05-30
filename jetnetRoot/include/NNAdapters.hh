#ifndef NN_ADAPTERS
#define NN_ADAPTERS

class JetNet; 
class TFlavorNetwork; 

#include <string> 

TFlavorNetwork* getTrainedNetwork(const JetNet&); 

void setTrainedNetwork(JetNet&, const TFlavorNetwork* trainedNetwork); 

TFlavorNetwork* getOldTrainedNetwork(std::string file_name); 

template<class O, class I>
O convert_node(const I& in) { 
  O out; 
  out.name = in.name; 
  out.offset = in.offset; 
  out.scale = in.scale; 
  return out; 
}


#endif // NN_ADAPTERS
