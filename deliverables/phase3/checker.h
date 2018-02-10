# ifndef CHECKER_H
# define CHECKER_H
# include "Type.h"
# include "Scope.h"

using namespace std;

void openScope(string message);
void closeScope(string message);

Symbol *defineFunction(const std::string &name, const Type &type, int line_number);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
void *useSymbol(string name, Type type);
Type checkVoid(const string name, const Type &type);

# endif