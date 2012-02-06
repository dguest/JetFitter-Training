#ifndef PY_NN_H
#define PY_NN_H

class ParseException {}; 
class TupleParseException: public ParseException{}; 
class IntParseException: public ParseException{};


std::vector<int> parse_int_tuple(PyObject* py_list); 


#endif 
