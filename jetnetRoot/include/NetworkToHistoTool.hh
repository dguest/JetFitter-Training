//-*-c++-*-
#ifndef NETWORK_TO_HISTO_TOOL_H
#define NETWORK_TO_HISTO_TOOL_H

#include <string>
#include <map>

class TH1;
class TTrainedNetwork;


class NetworkToHistoTool 
{
public:
  
  NetworkToHistoTool() {};

  ~NetworkToHistoTool() {};

  std::map<std::string,TH1*> histsFromNetwork(const TTrainedNetwork*) const;
  
  TTrainedNetwork* networkFromHists(std::map<std::string,TH1*>&) const;

private:
  
  
};


#endif
  
  
  
