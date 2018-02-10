# ifndef CHECKER_H
# define CHECKER_H
# include "Type.h"
# include "Scope.h"

using namespace std;

Scope openScope(string message);
Scope closeScope(string message);

Symbol *defineFunction(const std::string &name, const Type &type, int line_number);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);
Symbol *checkFunction(const std::string &name);
Symbol *useSymbol(string name, Type type);
Type checkVoid(const string name, const Type &type);

# endif