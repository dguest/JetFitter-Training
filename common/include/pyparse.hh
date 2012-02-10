#ifndef PYPARSE_H
#define PYPARSE_H

#include <vector>
#include <string> 
#include <Python.h>

class ParseException {}; 

class ListParseException: public ParseException{}; 
class TupleParseException: public ParseException{}; 

class StringParseException: public ParseException{};
class IntParseException: public ParseException{};
class FloatParseException: public ParseException{}; 


std::vector<std::string> parse_string_list(PyObject* string_list);
std::vector<int> parse_int_tuple(PyObject* py_list); 
std::vector<double> parse_double_list(PyObject* py_list); 


#endif // PYPARSE_H
