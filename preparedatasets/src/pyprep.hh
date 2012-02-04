#ifndef PY_PREP_H
#define PY_PREP_H

class ParseException {}; 
class ListParseException: public ParseException{}; 
class StringParseException: public ParseException{};

std::vector<std::string> parse_string_list(PyObject* string_list);



#endif 
