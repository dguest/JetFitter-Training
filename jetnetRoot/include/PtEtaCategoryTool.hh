#ifndef PT_ETA_CATEGORY_TOOL_H
#define PT_ETA_CATEGORY_TOOL_H

#include <map>
#include <vector>
#include <ostream>

class CategoryRange
{
public: 
  CategoryRange(double min, double max); 
  CategoryRange(double one_point); 
  bool operator<(const CategoryRange& r) const ; 
  friend std::ostream& operator<<(std::ostream&, const CategoryRange&); 
private: 
  double _min; 
  double _max; 
}; 

class CategoryMap: public std::map<CategoryRange,int> 
{ 
public: 
  CategoryMap(const std::vector<double>& v); 
  int get_category(double value); 

}; 

std::ostream& operator<<(std::ostream& out, const CategoryRange& range); 

#endif //PT_ETA_CATEGORY_TOOL_H
