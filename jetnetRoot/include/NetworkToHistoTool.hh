//-*-c++-*-
#ifndef __TNetworkToHistoTool_
#define __TNetworkToHistoTool_

#include <string>
#include <map>

class TH1;
class TNeuralNetwork;


class NetworkToHistoTool 
{
public:
  
  NetworkToHistoTool() {};

  ~NetworkToHistoTool() {};

  std::map<std::string,TH1*> histsFromNetwork(const TNeuralNetwork*) const;
  
  TNeuralNetwork* networkFromHists(std::map<std::string,TH1*>&) const;

private:
  
  
};


#endif
  
  
  
