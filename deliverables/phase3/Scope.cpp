# include "Scope.h"

// Constructor
Scope::Scope(Scope *enclosing) : _enclosing(enclosing){}

// Getters
const Symbols &Scope::getSymbols() const {return _symbols;}
Scope *Scope::getEnclosing() const {return _enclosing;}

// Logical Functions
void Scope::insert(Symbol *symbol){_symbols.push_back(symbol);}

Symbol *Scope::find(const string &name) const {
    for (int i = 0; i < _symbols.size(); i++)
	if (_symbols[i]->getName() == name){
        return getSymbols()[i];
    }
    return NULL;
}

Symbol *Scope::lookup(const string &name) const{
    Scope *parent = getEnclosing();

	while(parent != NULL){                      // Did not reach outermost scope yet
		if(parent->find(name) != NULL){         // Serach your current scope
			return parent->find(name);          // Found it! 
		}
		parent = parent->getEnclosing();           // Did not find it in your current scope. go out
		
	}
	return NULL;                                // Did not find 
}
