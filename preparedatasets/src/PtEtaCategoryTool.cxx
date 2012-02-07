#include "PtEtaCategoryTool.hh"
#include <map> 
#include <vector>
#include <cassert>
#include <utility>
#include <cmath>

CategoryMap::CategoryMap(const std::vector<float>& v)
{ 
  float low = -INFINITY; 
  float high = low; 
  for (size_t category = 0; category < v.size(); category++){ 
    low = high;
    high = v.at(category); 
    CategoryRange the_range(low,high); 
    this->insert(std::make_pair(the_range, category)); 
  }

  CategoryRange end_range(high, INFINITY); 
  this->insert( std::make_pair(end_range,v.size())); 
  
}

std::ostream& operator<<(std::ostream& out, const CategoryRange& range)
{
  out << range._min << " -- " << range._max; 
  return out; 
} 


int CategoryMap::get_category(float value)
{
  CategoryRange r(value,value);// + fabs(value*1e-5)); 
  std::map<CategoryRange,int>::iterator loc = this->find(r); 
  if (loc == this->end())
    return -1; 
  else return loc->second; 
}

CategoryRange::CategoryRange(float min, float max): 
  _min(min), 
  _max(max)
{ 
  assert(min <= max); 
}


bool CategoryRange::operator<(const CategoryRange& r) const
{
  if (this->_max <= r._max){ 
    if (this->_min < r._min) 
      return true; 
  }
  return false; 
}
