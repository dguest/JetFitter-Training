#ifndef NN_EXCEPTIONS_H
#define NN_EXCEPTIONS_H

class NNException {}; 

class LoadReducedDSException: public NNException {}; 

class LoadNetworkException: public NNException {};
class WrongNetworkSizeException: public LoadNetworkException {}; 

class WriteFileException: public NNException {};



class ParseException {}; 
class TupleParseException: public ParseException{}; 
class IntParseException: public ParseException{};



#endif // NN_EXCEPTIONS_H
