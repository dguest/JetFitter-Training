#ifndef UNIT_TESTS_H
#define UNIT_TESTS_H

#include <iostream>
#include <vector>

bool test_trained(); 

float get_rand(float range = 1, float offset = 0); 

template<class T>
void print_node_info(T container) { 
  for (typename T::const_iterator itr = 
	 container.begin();
       itr != container.end(); 
       itr++){ 
    std::cout << itr->name << " -- o: " << itr->offset 
	      << " s: " << itr->scale << "\n"; 
  }
}

#endif // UNIT_TESTS_H
