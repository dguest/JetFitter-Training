#ifndef BIN_TOOL_H
#define BIN_TOOL_H

#include <map>
#include <vector>
#include <ostream>


class BinTool: public std::map<double,int> 
{ 
public: 
  BinTool(const std::vector<double>& v); 
  int get_bin(double value); 
}; 


#endif //BIN_TOOL_H
