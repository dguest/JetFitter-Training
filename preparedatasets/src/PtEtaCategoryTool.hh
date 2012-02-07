#ifndef PT_ETA_CATEGORY_TOOL_H
#define PT_ETA_CATEGORY_TOOL_H

#include <map>
#include <vector>
#include <ostream>

class CategoryRange
{
public: 
  CategoryRange(float min, float max); 
  CategoryRange(float one_point); 
  bool operator<(const CategoryRange& r) const ; 
  friend std::ostream& operator<<(std::ostream&, const CategoryRange&); 
private: 
  float _min; 
  float _max; 
}; 

class CategoryMap: public std::map<CategoryRange,int> 
{ 
public: 
  CategoryMap(const std::vector<float>& v); 
  int get_category(float value); 

}; 

std::ostream& operator<<(std::ostream& out, const CategoryRange& range); 

#endif //PT_ETA_CATEGORY_TOOL_H
