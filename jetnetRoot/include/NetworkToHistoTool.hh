//-*-c++-*-
#ifndef __TNetworkToHistoTool_
#define __TNetworkToHistoTool_

#include <string>
#include <vector>

class TH1;
class TFlavorNetwork;


class NetworkToHistoTool 
{
public:
  
  NetworkToHistoTool() {};

  ~NetworkToHistoTool() {};

  std::vector<TH1*> fromTrainedNetworkToHisto(const TFlavorNetwork*) const;
  
  TFlavorNetwork* fromHistoToTrainedNetwork(std::vector<TH1*> &) const;

private:

  TH1* findHisto(std::string nameOfHisto,
		 std::vector<TH1*> & inputHistos) const;
  
  
};


#endif
  
  
  
