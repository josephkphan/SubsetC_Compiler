# include "Symbol.h"

using std::string;

// Constructor
Symbol::Symbol(const string &name, const Type &type) : _name(name), _type(type){_definedFlag = 0;}

// Getter
const string &Symbol::getName() const{return _name;}
const Type &Symbol::getType() const{return _type;}

