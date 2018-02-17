/*
 * File:	checker.cpp
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the semantic checker for Simple C.
 *
 *		If a symbol is redeclared, the existing declaration is
 *		retained and the redeclaration discarded.  This behavior
 *		seems to be consistent with GCC, and who are we to argue
 *		with GCC?
 *
 *		Extra functionality:
 *		- inserting an undeclared symbol with the error type
 */

# include <iostream>
# include "lexer.h"
# include "checker.h"
# include "nullptr.h"
# include "tokens.h"
# include "Symbol.h"
# include "Scope.h"
# include "Type.h"

# define FUNCDEFN 1


using namespace std;

static Scope *outermost, *toplevel;
static const Type error;

static string redefined = "redefinition of '%s'";
static string redeclared = "redeclaration of '%s'";
static string conflicting = "conflicting types for '%s'";
static string undeclared = "'%s' undeclared";
static string void_object = "'%s' has type void";

static string invalid_return_type = "invalid return type";
static string invalid_test_type = "invalid type for test expression";
static string lvalueReq = "lvalue required in expression";
static string invBinary = "invalid operands to binary %s";
static string invUnary = "invalid operands to unary %s";
static string notFunction = "called object is not a function";
static string invArgs = "invalid arguments to called function";

/*
 * Function:	checkIfVoidObject
 *
 * Description:	Check if TYPE is a proper use of the void type (if the
 *		specifier is void, then the indirection must be nonzero or
 *		the kind must be a function).  If the type is proper, it is
 *		returned.  Otherwise, the error type is returned.
 */

static Type checkIfVoidObject(const string name, const Type &type)
{
    if (type.specifier() != VOID)
	return type;

    if (type.indirection() == 0 && !type.isFunction()) {
	report(void_object, name);
	return error;
    }

    return type;
}


/*
 * Function:	openScope
 *
 * Description:	Create a scope and make it the new top-level scope.
 */

Scope *openScope()
{
    toplevel = new Scope(toplevel);

    if (outermost == nullptr)
	outermost = toplevel;

    return toplevel;
}


/*
 * Function:	closeScope
 *
 * Description:	Remove the top-level scope, and make its enclosing scope
 *		the new top-level scope.
 */

Scope *closeScope()
{
    Scope *old = toplevel;
    toplevel = toplevel->enclosing();
    return old;
}


/*
 * Function:	defineFunction
 *
 * Description:	Define a function with the specified NAME and TYPE.  A
 *		function is always defined in the outermost scope.
 */

Symbol *defineFunction(const string &name, const Type &type)
{
    Symbol *symbol = declareFunction(name, type);

    if (symbol->_attributes & FUNCDEFN)
	report(redefined, name);

    symbol->_attributes = FUNCDEFN;
    return symbol;
}


/*
 * Function:	declareFunction
 *
 * Description:	Declare a function with the specified NAME and TYPE.  A
 *		function is always declared in the outermost scope.  Any
 *		redeclaration is discarded.
 */

Symbol *declareFunction(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = outermost->find(name);

    if (symbol == nullptr) {
	symbol = new Symbol(name, type);
	outermost->insert(symbol);

    } else if (type != symbol->type()) {
	report(conflicting, name);
	delete type.parameters();

    } else
	delete type.parameters();

    return symbol;
}


/*
 * Function:	declareVariable
 *
 * Description:	Declare a variable with the specified NAME and TYPE.  Any
 *		redeclaration is discarded.
 */

Symbol *declareVariable(const string &name, const Type &type)
{
    cout << name << ": " << type << endl;
    Symbol *symbol = toplevel->find(name);

    if (symbol == nullptr) {
	symbol = new Symbol(name, checkIfVoidObject(name, type));
	toplevel->insert(symbol);

    } else if (outermost != toplevel)
	report(redeclared, name);

    else if (type != symbol->type())
	report(conflicting, name);

    return symbol;
}


/*
 * Function:	checkIdentifier
 *
 * Description:	Check if NAME is declared.  If it is undeclared, then
 *		declare it as having the error type in order to eliminate
 *		future error messages.
 */

Symbol *checkIdentifier(const string &name)
{
    Symbol *symbol = toplevel->lookup(name);

    if (symbol == nullptr) {
	report(undeclared, name);
	symbol = new Symbol(name, error);
	toplevel->insert(symbol);
    }

    return symbol;
}



// ------------------ Type Checker Funtions -------------------- // 

void debug_log(string message){
	cout << "*****DEBUG - Checker: " << message << endl;
}

bool type_error_check(const Type &left, const Type &right){
    if(left == Type() || right == Type()){
        return true;
    }else{
        return false;
    }
}

// Logical Operators
// Possible Error Codes: || or &&
Type checkLogicalOperator(const Type &left, const Type &right, const string error_code){
    if(type_error_check(left, right)){return Type();}   

    Type left_type = left.promote();
    Type right_type = right.promote();

    if(left_type.isPredicate() && right_type.isPredicate()){
        return Type(INT);
    }else {
        report(invBinary, error_code); 
        return Type();
    }
}


// Comparison Operators
// Possible Error Codes: <, <= >, >= 
Type checkComparisonOperator(const Type &left, const Type &right, const string &error_code){
    if(type_error_check(left, right)){return Type();}     

    Type left_type = left.promote();
    Type right_type = right.promote();

    if(left_type == right_type && left_type.isPredicate()){   
        return Type(INT);
    }else{
        report(invBinary, error_code);
        return Type();
    }
}

// Equality Operators
// Possible Error Codes: ==, !=
Type checkEqualityOperator(const Type &left, const Type &right, const string &error_code){
    if(type_error_check(left, right)){return Type();}   

    Type left_type = left.promote();
    Type right_type = right.promote();

    if(left_type.isCompatible(left_type, right_type)){   
        return Type(INT);
    }else{
        report(invBinary, error_code);
        return Type();
    }
}


// Math Operators
// Possible Error Codes: *, /, %, +, -
Type checkAdd(const Type &left_type, const Type &right_type, const string &error_code){
    if(left_type.isInteger() && right_type.isInteger()){
        return Type(INT);
    }else if(left_type.isPointer() && !left_type.isPtrToNull() && right_type.isInteger()){
        return left_type;
    }else if(right_type.isPointer() && !right_type.isPtrToNull() && left_type.isInteger()){
        return right_type;
    }else {
        report(invBinary, error_code);
        return Type();
    }
}

Type checkSub(const Type &left_type, const Type &right_type, const string &error_code){
    if(left_type.isInteger() && right_type.isInteger()){
        return Type(INT);
    }else if(left_type.isPointer() && !left_type.isPtrToNull() && right_type.isInteger()){
        return left_type;
    }else if(left_type.isPointer() && !left_type.isPtrToNull() && right_type.isPointer() && !right_type.isPtrToNull()){
        if(right_type == left_type){
            return Type(INT);
        }else{
            report(invBinary, error_code);
            return Type();
        }
    }else {
        report(invBinary, error_code);
        return Type(); 
    }
}

Type checkMathOperator(const Type &left, const Type &right, const string &error_code){
    if(type_error_check(left, right)){return Type();}    

    Type left_type = left.promote();
    Type right_type = right.promote();

    // Jump to Add Function
    if(error_code == "+"){
        return checkAdd(left_type, right_type, error_code); 
    }
    
    // Jump to Subtract Function
    if (error_code == "-"){
        return checkSub(left_type, right_type, error_code); 
    }

    // All other Operators
    if(left_type.isInteger() && right_type.isInteger()){   
        return Type(INT); 
    }else{
        report(invBinary, error_code);
        return Type();
    }
}

// Uniary Operators 

Type checkNeg(const Type &left){
    if(left == Type()){ return Type(); }    

    if(left.isInteger()){
        return left;
    }else{
        report(invUnary, "-");
        return Type();
    }
}

Type checkNot(const Type &left){
    if(left == Type()){ return Type(); }   

    if(left.isPredicate()){
        return Type(INT);
    }else{
        report(invUnary, "!");
        return Type();
    }
}
Type checkAddr(const Type &left, bool &lvalue){
    if(left == Type()){ return Type(); }   

    if(!lvalue){
        report(lvalueReq);
        return Type();
    }else{
        return Type(left.specifier(), left.indirection()+1);
    }
}

Type checkDeref(const Type &left){
    if(left == Type()){ return Type(); }   
    
    Type left_type = left.promote();
    if(left_type.isPointer() && !left_type.isPtrToNull()){
        left_type = Type(left_type.specifier(), left_type.indirection()-1);
        return left_type;
    }else{
        report(invUnary, "*");
        return Type();
    }
}

Type checkSizeof(const Type &left){
    if(left == Type()){ return Type(); }   

    if(left.isPredicate()){
        return Type(INT);
    }else{
        report(invUnary, "sizeof");
        return Type();
    }
}

