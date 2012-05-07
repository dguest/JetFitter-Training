//-*-c++-*-
#ifndef __TNetworkToHistoTool_
#define __TNetworkToHistoTool_

#include <string>
#include <vector>

class TH1;
class TTrainedNetwork;

//by Giacinto Piacquadio (19-2-2008)

class NetworkToHistoTool // : public TObject 
{
public:
  
  NetworkToHistoTool() {};

  ~NetworkToHistoTool() {};

  std::vector<TH1*> fromTrainedNetworkToHisto(TTrainedNetwork*) const;
  
  TTrainedNetwork* fromHistoToTrainedNetwork(std::vector<TH1*> &) const;

private:

  TH1* findHisto(std::string nameOfHisto,
		 std::vector<TH1*> & inputHistos) const;
  
  //  ClassDef( TNetworkToHistoTool, 1 );
  
};


#endif
  
  
  
