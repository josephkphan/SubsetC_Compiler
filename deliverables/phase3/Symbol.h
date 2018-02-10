# ifndef SYMBOL_H
# define SYMBOL_H
# include "Type.h"

class Symbol {
    typedef std::string string;
    string _name;
    Type _type;
    
public:
    int _definedFlag;

    // Constructor
    Symbol(const string &name, const Type &type);

    // Getters
    const string &getName() const;
    const Type &getType() const;

};

# endif