#ifndef NN_ADAPTERS
#define NN_ADAPTERS

class JetNet; 
class TTrainedNetwork; 

TTrainedNetwork* getTrainedNetwork(const JetNet&); 

void setTrainedNetwork(JetNet&, const TTrainedNetwork* trainedNetwork); 


template<class O, class I>
O convert_node(const I& in) { 
  O out; 
  out.name = in.name; 
  out.offset = in.offset; 
  out.scale = in.scale; 
  return out; 
}


#endif // NN_ADAPTERS
