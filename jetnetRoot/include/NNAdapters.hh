#ifndef NN_ADAPTERS
#define NN_ADAPTERS

class JetNet; 
class TOldNetwork; 

#include "TTrainedNetwork.h"

#include <string> 
#include <vector>

TTrainedNetwork* getTrainedNetwork(const JetNet&); 

void setTrainedNetwork(JetNet&, const TTrainedNetwork* trainedNetwork); 

TTrainedNetwork* getOldTrainedNetwork(std::string file_name); 
TTrainedNetwork* convertOldToNew(const TOldNetwork* old,
				std::vector<TTrainedNetwork::Input>); 
TOldNetwork* convertNewToOld(const TTrainedNetwork* new_net); 

template<class O, class I>
O convert_node(const I& in) { 
  O out; 
  out.name = in.name; 
  out.offset = in.offset; 
  out.scale = in.scale; 
  return out; 
}


#endif // NN_ADAPTERS
