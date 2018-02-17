/*
 * File:	checker.h
 *
 * Description:	This file contains the public function declarations for the
 *		semantic checker for Simple C.
 */

# ifndef CHECKER_H
# define CHECKER_H
# include "Scope.h"

Scope *openScope();
Scope *closeScope();

Symbol *defineFunction(const std::string &name, const Type &type);
Symbol *declareFunction(const std::string &name, const Type &type);
Symbol *declareVariable(const std::string &name, const Type &type);
Symbol *checkIdentifier(const std::string &name);

Type checkFuncCall(const Type &funcType, const Parameters &args);

// New Functions Added
// Binary Operators
Type checkLogicalOperator(const Type &left, const Type &right, const std::string &error_code);
Type checkEqualityOperator(const Type &left, const Type &right, const std::string &error_code);
Type checkComparisonOperator(const Type &left, const Type &right, const std::string &error_code);
Type checkMathOperator(const Type &left, const Type &right, const std::string &error_code);

// Uniary Operators 
Type checkNeg(const Type &left);
Type checkNot(const Type &left);
Type checkAddr(const Type &left, bool &lvalue);
Type checkDeref(const Type &left);
Type checkSizeof(const Type &left);
# endif /* CHECKER_H */

