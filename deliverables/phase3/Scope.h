# ifndef SCOPE_H
# define SCOPE_H
# include "Symbol.h"
# include <vector>

typedef std::vector<Symbol *> Symbols;

class Scope {
    typedef std::string string;
    Scope *_enclosing;
    Symbols _symbols;

public:
    // Constructor
    Scope(Scope *enclosing=0);

    // Getters
    Scope *getEnclosing() const;
    const Symbols &getSymbols() const;

    // Logic Functions
    void insert(Symbol *symbol);
    Symbol *lookup(const string &name) const;
    Symbol *find(const string &name) const;
};

#endif

