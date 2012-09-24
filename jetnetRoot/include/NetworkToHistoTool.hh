//-*-c++-*-
#ifndef __TNetworkToHistoTool_
#define __TNetworkToHistoTool_

#include <string>
#include <map>

class TH1;
class TFlavorNetwork;


class NetworkToHistoTool 
{
public:
  
  NetworkToHistoTool() {};

  ~NetworkToHistoTool() {};

  std::map<std::string,TH1*> histsFromNetwork(const TFlavorNetwork*) const;
  
  TFlavorNetwork* networkFromHists(std::map<std::string,TH1*>&, 
				   unsigned options = 0) const;

private:
  
  
};


#endif
  
  
  
