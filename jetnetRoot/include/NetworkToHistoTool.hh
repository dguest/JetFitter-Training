//-*-c++-*-
#ifndef NETWORK_TO_HISTO_TOOL_H
#define NETWORK_TO_HISTO_TOOL_H

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
  
  
  
