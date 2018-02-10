#include <iostream>
#include <string>
#include <vector>
#include "tokens.h"
#include "checker.h"
#include "lexer.h"
#include "Type.h"
#include "Symbol.h"
#include "Scope.h"

using namespace std;

static const Type error;
static string redefinition = "redefinition of '%s'";
static string redeclaration = "redeclaration of '%s'";
static string undeclared = "'%s' undeclared";
static string voidType = "'%s' has type void";
static string conflictingType = "conflicting types for '%s'";

Scope *current_scope, *global_scope;

void debug_message(string message){
    // cout << "*****DEBUG: " << message << endl;
}

void openScope(string message){
	current_scope = new Scope(current_scope);
	debug_message("Creating Scope" + message);
	if(global_scope == NULL){
		global_scope = current_scope;
	}
}

void closeScope(string message){
	Scope *parent_scope;
	parent_scope = current_scope;
	if(current_scope != NULL){
		current_scope = current_scope->getEnclosing();
		debug_message("Closing Scope"+ message);
	}
}

Symbol *defineFunction(const string &name, const Type &type, int line_number){
    Symbol *symbol = declareFunction(name, type); 

    if (symbol->_definedFlag != 0){
		cerr << "line " << line_number << ": " << "redefinition of '"<<name <<"'" << endl;
	}
	symbol->_definedFlag = 1;
	return symbol;
}


Symbol *declareFunction(const string &name, const Type &type){
    // cout << name << ": " << type << endl;
    Symbol *symbol = global_scope->find(name);

    if (symbol == NULL) {
		symbol = new Symbol(name, type);
		global_scope->insert(symbol);
    } else if (type != symbol->getType()) {
		report(conflictingType, name);
    }
    return symbol;
}

Symbol *declareVariable(const string &name, const Type &type){
    // cout << name << ": " << type << endl;
    Symbol *symbol = current_scope->find(name);

    if (symbol == NULL) {
		Type t = checkVoid(name, type);
		symbol = new Symbol(name, t);
		current_scope->insert(symbol);
    } else if (global_scope != current_scope){
		report(redeclaration, name);
	}else if (type != symbol->getType()){
		report(conflictingType, name);
	}
    return symbol;
}

void *useSymbol(string name, Type type){
	// cout << "Use symbol 1: Entry" << name << endl;

	Symbol *symbol;
	symbol = current_scope->find(name);

	if(symbol == NULL){
		// cout << "Use symbol 2:  " << name << ": " << type << " NOT found in CURRENT scope" << endl;
		symbol = current_scope->lookup(name);
		if(symbol == NULL){
			// cout << "Use symbol 3: symbol " << name << ": " << type << " Was an Undeclared Bitch" << endl;
			report(undeclared, name);
		}else {
			// cout << "Use symbol 3: symbol " << name << ": " << type << " FOUND in OUTER scope" << endl;
		}
		
	}else {
		// cout << "Use symbol 2:  " << name << ": " << type << " FOUND in CURRENT scope" << endl;
	}
} 

Type checkVoid(const string name, const Type &type){
    if (type.getSpecifier() != VOID){
		return type;
	}else if (type.getIndirection() == 0 && !(type.isFunction()) )  {
		report(voidType, name);
		return error;
    }else{
		return type;
	}
}
