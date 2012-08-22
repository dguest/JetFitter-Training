#ifndef PY_PREP_H
#define PY_PREP_H

#include <string> 

const unsigned MAX_DOC_STRING_LENGTH = 1000; 
void build_doc(char* doc_array, std::string before, 
	       const char** input_kwds, std::string after); 

#endif 
