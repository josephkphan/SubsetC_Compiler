/*
 * File:	parser.c
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the recursive-descent parser for
 *		Simple C.
 */

# include <cstdlib>
# include <iostream>
# include "checker.h"
# include "tokens.h"
# include "lexer.h"

using namespace std;

static int lookahead;
static Type expression(bool &lvalue);
static void statement();
static int loop_counter = 0;

void parser_debug(string message){
    cout << "--" << message << endl;
}
/*
 * Function:	error
 *
 * Description:	Report a syntax error to standard error.
 */

static void error()
{
    if (lookahead == DONE)
	report("syntax error at end of file");
    else
	report("syntax error at '%s'", yytext);

    exit(EXIT_FAILURE);
}


/*
 * Function:	match
 *
 * Description:	Match the next token against the specified token.  A
 *		failure indicates a syntax error and will terminate the
 *		program since our parser does not do error recovery.
 */

static void match(int t)
{
    if (lookahead != t)
	error();

    lookahead = yylex();
}


/*
 * Function:	expect
 *
 * Description:	Match the next token against the specified token, and
 *		return its lexeme.  We must save the contents of the buffer
 *		from the lexical analyzer before matching, since matching
 *		will advance to the next token.
 */

static string expect(int t)
{
    string buf = yytext;
    match(t);
    return buf;
}


/*
 * Function:	number
 *
 * Description:	Match the next token as a number and return its value.
 */

static unsigned number()
{
    int value;


    value = strtoul(expect(NUM).c_str(), NULL, 0);
    return value;
}


/*
 * Function:	isSpecifier
 *
 * Description:	Return whether the given token is a type specifier.
 */

static bool isSpecifier(int token)
{
    return token == INT || token == CHAR || token == VOID;
}


/*
 * Function:	specifier
 *
 * Description:	Parse a type specifier.  Simple C has only ints, chars, and
 *		void types.
 *
 *		specifier:
 *		  int
 *		  char
 *		  void
 */

static int specifier()
{
    int typespec = ERROR;


    if (isSpecifier(lookahead)) {
	typespec = lookahead;
	match(lookahead);
    } else
	error();

    return typespec;
}


/*
 * Function:	pointers
 *
 * Description:	Parse pointer declarators (i.e., zero or more asterisks).
 *
 *		pointers:
 *		  empty
 *		  * pointers
 */

static unsigned pointers()
{
    unsigned count = 0;


    while (lookahead == '*') {
	match('*');
	count ++;
    }

    return count;
}


/*
 * Function:	declarator
 *
 * Description:	Parse a declarator, which in Simple C is either a scalar
 *		variable or an array, with optional pointer declarators.
 *
 *		declarator:
 *		  pointers identifier
 *		  pointers identifier [ num ]
 */

static void declarator(int typespec)
{
    unsigned indirection;
    string name;


    indirection = pointers();
    name = expect(ID);

    if (lookahead == '[') {
	match('[');
	declareVariable(name, Type(typespec, indirection, number()));
	match(']');
    } else
	declareVariable(name, Type(typespec, indirection));
}


/*
 * Function:	declaration
 *
 * Description:	Parse a local variable declaration.  Global declarations
 *		are handled separately since we need to detect a function
 *		as a special case.
 *
 *		declaration:
 *		  specifier declarator-list ';'
 *
 *		declarator-list:
 *		  declarator
 *		  declarator , declarator-list
 */

static void declaration()
{
    int typespec;


    typespec = specifier();
    declarator(typespec);

    while (lookahead == ',') {
	match(',');
	declarator(typespec);
    }

    match(';');
}


/*
 * Function:	declarations
 *
 * Description:	Parse a possibly empty sequence of declarations.
 *
 *		declarations:
 *		  empty
 *		  declaration declarations
 */

static void declarations()
{
    while (isSpecifier(lookahead))
	declaration();
}


/*
 * Function:	primaryExpression
 *
 * Description:	Parse a primary expression.
 *
 *		primary-expression:
 *		  ( expression )
 *		  identifier ( expression-list )
 *		  identifier ( )
 *		  identifier
 *		  character
 *		  string
 *		  num
 *
 *		expression-list:
 *		  expression
 *		  expression , expression-list
 */

static Type primaryExpression(bool &lvalue){
	Type left;	
	string name;
	Parameters parameters;
	Symbol *function_symbol;
    if (lookahead == '(') {
		match('(');
		left = expression(lvalue);
		match(')');
		lvalue=false;

    } else if (lookahead == CHARACTER) {
		name = expect(CHARACTER);
		left = Type(CHAR, 0);
		lvalue=false;

    } else if (lookahead == STRING) {
		name = expect(STRING);
		int length = name.length() - 2;
		left = Type(CHAR, 0, length);

		lvalue=false;

    } else if (lookahead == NUM) {
		match(NUM);
		left = Type(INT);
		lvalue=false;

    } else if (lookahead == ID) {
		name = expect(ID);
		// checkIdentifier(expect(ID));
		lvalue=false;

		if (lookahead == '(') {
			parser_debug("primaryExpression: ID if happend");
			match('(');

			if (lookahead != ')') {
				parameters._types.push_back(expression(lvalue));

				while (lookahead == ',') {
					match(',');
					parameters._types.push_back(expression(lvalue));
				}
			}

			match(')');
			function_symbol=checkFunction(name);
			// checkFunctionParameters(function_symbol,parameters);
			left = checkFuncCall(function_symbol->type(),parameters);
			parser_debug("primaryExpression: return in ID");
			parser_debug("primaryExpression: LEFT TYPE - " + left.toString());
			return left;
		}else {
			Symbol *checkID = checkIdentifier(name);
		    left = checkID->type();
		    //cout << "IN PRIMARYEXPRESSION ID ELSE" << endl;
		    if(checkID->type().isScalar()){
		    	lvalue = true;
		    }	
		    else{
		    	lvalue = false;
		    }
		return left;
		}


    } else{
		parser_debug("primaryExpression: return error");
		error();
		return Type();
	}

	return left;
}


/*
 * Function:	postfixExpression
 *
 * Description:	Parse a postfix expression.
 *
 *		postfix-expression:
 *		  primary-expression
 *		  postfix-expression [ expression ]
 */

static Type postfixExpression(bool &lvalue)
{
    Type left = primaryExpression(lvalue);
	Type right;
    while (lookahead == '[') {
		match('[');
		right = expression(lvalue);
		left = checkArray(left,right);  
		match(']');
		lvalue = true;
	}
	
	return left;
}


/*
 * Function:	prefixExpression
 *
 * Description:	Parse a prefix expression.
 *
 *		prefix-expression:
 *		  postfix-expression
 *		  ! prefix-expression
 *		  - prefix-expression
 *		  * prefix-expression
 *		  & prefix-expression
 *		  sizeof prefix-expression
 */

static Type prefixExpression(bool &lvalue)
{
	Type left;
    if (lookahead == '!') {
		match('!');
		left = prefixExpression(lvalue);
		left = checkNot(left);
		lvalue = false;

    } else if (lookahead == '-') {
		match('-');
		left = prefixExpression(lvalue);
		left = checkNeg(left);
		lvalue = false;

    } else if (lookahead == '*') {
		match('*');
		left = prefixExpression(lvalue);
		left = checkDeref(left);
		lvalue = true;

    } else if (lookahead == '&') {
		match('&');
		left = prefixExpression(lvalue);
		left = checkAddr(left,lvalue);
		lvalue = false;

    } else if (lookahead == SIZEOF) {
		match(SIZEOF);
		left = prefixExpression(lvalue);
		left = checkSizeof(left);
		lvalue = false;

    } else{
		left = postfixExpression(lvalue);
	}

	return left;

}


/*
 * Function:	multiplicativeExpression
 *
 * Description:	Parse a multiplicative expression.  Simple C does not have
 *		cast expressions, so we go immediately to prefix
 *		expressions.
 *
 *		multiplicative-expression:
 *		  prefix-expression
 *		  multiplicative-expression * prefix-expression
 *		  multiplicative-expression / prefix-expression
 *		  multiplicative-expression % prefix-expression
 */

static Type multiplicativeExpression(bool &lvalue)
{
	Type left, right;
    left = prefixExpression(lvalue);

    while (1) {
	if (lookahead == '*') {
	    match('*');
		right = prefixExpression(lvalue);
		left = checkMathOperator(left,right,"*");
		lvalue = false;

	} else if (lookahead == '/') {
	    match('/');
		right = prefixExpression(lvalue);
		left = checkMathOperator(left,right,"/");
		lvalue = false;

	} else if (lookahead == '%') {
	    match('%');
		right = prefixExpression(lvalue);
		left = checkMathOperator(left,right,"%");
		lvalue = false;

	} else
	    break;
	}
	return left;
}


/*
 * Function:	additiveExpression
 *
 * Description:	Parse an additive expression.
 *
 *		additive-expression:
 *		  multiplicative-expression
 *		  additive-expression + multiplicative-expression
 *		  additive-expression - multiplicative-expression
 */

static Type additiveExpression(bool &lvalue)
{
	Type left, right;
	left = multiplicativeExpression(lvalue);

    while (1) {
	if (lookahead == '+') {
	    match('+');
		right = multiplicativeExpression(lvalue);
		left = checkMathOperator(left,right,"+");
		lvalue = false;

	} else if (lookahead == '-') {
	    match('-');
	    right = multiplicativeExpression(lvalue);
		left = checkMathOperator(left,right,"-");
		lvalue = false;
	} else
	    break;
	}
	return left;
}


/*
 * Function:	relationalExpression
 *
 * Description:	Parse a relational expression.  Note that Simple C does not
 *		have shift operators, so we go immediately to additive
 *		expressions.
 *
 *		relational-expression:
 *		  additive-expression
 *		  relational-expression < additive-expression
 *		  relational-expression > additive-expression
 *		  relational-expression <= additive-expression
 *		  relational-expression >= additive-expression
 */

static Type relationalExpression(bool &lvalue)
{
	Type left, right;
    left = additiveExpression(lvalue); 

    while (1) {
		if (lookahead == '<') {
			match('<');
			right = additiveExpression(lvalue);
			left = checkComparisonOperator(left,right,"<");
			lvalue = false;
		} else if (lookahead == '>') {
			match('>');
			right = additiveExpression(lvalue);
			left = checkComparisonOperator(left,right,">");
			lvalue = false;
		} else if (lookahead == LEQ) {
			match(LEQ);
			right = additiveExpression(lvalue);
			left = checkComparisonOperator(left,right,"<=");
			lvalue = false;
		} else if (lookahead == GEQ) {
			match(GEQ);
			right = additiveExpression(lvalue);
			left = checkComparisonOperator(left,right,">=");
			lvalue = false;
		} else{
			break;
		}
	}
	
	return left;
}


/*
 * Function:	equalityExpression
 *
 * Description:	Parse an equality expression.
 *
 *		equality-expression:
 *		  relational-expression
 *		  equality-expression == relational-expression
 *		  equality-expression != relational-expression
 */

static Type equalityExpression(bool &lvalue)
{
	Type left, right;
    left = relationalExpression(lvalue);

    while (1) {
	if (lookahead == EQL) {
	    match(EQL);
		right = relationalExpression(lvalue);
		left = checkEqualityOperator(left,right,"==");
		lvalue = false;

	} else if (lookahead == NEQ) {
	    match(NEQ);
		right = relationalExpression(lvalue);
		left = checkEqualityOperator(left,right,"!=");
		lvalue = false;

	} else
	    break;
	}
	return left;
}


/*
 * Function:	logicalAndExpression
 *
 * Description:	Parse a logical-and expression.  Note that Simple C does
 *		not have bitwise-and expressions.
 *
 *		logical-and-expression:
 *		  equality-expression
 *		  logical-and-expression && equality-expression
 */

static Type logicalAndExpression(bool &lvalue)
{
	Type left, right;
    left = equalityExpression(lvalue);

    while (lookahead == AND) {
		match(AND);
		right = equalityExpression(lvalue);
		left = checkLogicalOperator(left,right,"&&");
		lvalue = false;
	}
	return left;
}


/*
 * Function:	expression
 *
 * Description:	Parse an expression, or more specifically, a logical-or
 *		expression, since Simple C does not allow comma or
 *		assignment as an expression operator.
 *
 *		expression:
 *		  logical-and-expression
 *		  expression || logical-and-expression
 */

static Type expression(bool &lvalue)
{
	Type left, right;
    left = logicalAndExpression(lvalue);

    while (lookahead == OR) {
		match(OR);
		right = logicalAndExpression(lvalue);
		left = checkLogicalOperator(left,right, "||");
		lvalue = false;	
	}
	return left;
}


/*
 * Function:	statements
 *
 * Description:	Parse a possibly empty sequence of statements.  Rather than
 *		checking if the next token starts a statement, we check if
 *		the next token ends the sequence, since a sequence of
 *		statements is always terminated by a closing brace.
 *
 *		statements:
 *		  empty
 *		  statement statements
 */

static void statements()
{
    while (lookahead != '}')
	statement();
}


/*
 * Function:	Assignment
 *
 * Description:	Parse an assignment statement.
 *
 *		assignment:
 *		  expression = expression
 *		  expression
 */

static void assignment()
{
	bool lvalue = false;
	bool rvalue = false;

    Type left = expression(lvalue);

    if (lookahead == '=') {
		match('=');
		Type right = expression(rvalue);
		checkAssignment(left,right,lvalue);				//TODO should check assignment have a lvalue
    }
}


/*
 * Function:	statement
 *
 * Description:	Parse a statement.  Note that Simple C has so few
 *		statements that we handle them all in this one function.
 *
 *		statement:
 *		  { declarations statements }
 *		  break ;
 *		  return expression ;
 *		  while ( expression ) statement
 *		  for ( assignment ; expression ; assignment ) statement
 *		  if ( expression ) statement
 *		  if ( expression ) statement else statement
 *		  assignment ;
 */

static void statement(){
	bool lvalue = false;
    if (lookahead == '{') {
		match('{');
		openScope();
		declarations();
		statements();
		closeScope();
		match('}');

    } else if (lookahead == BREAK) {
		match(BREAK);
		checkBreak(loop_counter);
		match(';');

    } else if (lookahead == RETURN) {
		match(RETURN);
		Type right = expression(lvalue);
		checkReturn(right);
		match(';');

    } else if (lookahead == WHILE) {
		match(WHILE);
		match('(');
		Type right = expression(lvalue);
		checkLoop(right);
		loop_counter++;
		checkTest(right);
		match(')');
		statement();
		loop_counter--;

    } else if (lookahead == FOR) {
		match(FOR);
		match('(');
		assignment();
		match(';');
		Type right = expression(lvalue); 
		checkLoop(right);						//TODO: 
		loop_counter++;
		checkTest(right);
		match(';');
		assignment();
		match(')');
		statement();
		loop_counter--;

    } else if (lookahead == IF) {
		match(IF);
		match('(');
		Type right = expression(lvalue);
		checkTest(right);
		match(')');
		statement();

	if (lookahead == ELSE) {
	    match(ELSE);
	    statement();
	}

    } else {
		assignment();
		match(';');
    }
}


/*
 * Function:	parameter
 *
 * Description:	Parse a parameter, which in Simple C is always a scalar
 *		variable with optional pointer declarators.
 *
 *		parameter:
 *		  specifier pointers identifier
 */

static Type parameter()
{
    int typespec;
    unsigned indirection;
    string name;
    Type type;


    typespec = specifier();
    indirection = pointers();
    name = expect(ID);

    type = Type(typespec, indirection);
    declareVariable(name, type);
    return type;
}


/*
 * Function:	parameters
 *
 * Description:	Parse the parameters of a function, but not the opening or
 *		closing parentheses.
 *
 *		parameters:
 *		  void
 *		  void pointers identifier remaining-parameters
 *		  char pointers identifier remaining-parameters
 *		  int pointers identifier remaining-parameters
 *
 *		remaining-parameters:
 *		  empty
 *		  , ...
 *		  , parameter remaining-parameters
 */

static Parameters *parameters()
{
    int typespec;
    unsigned indirection;
    Parameters *params;
    string name;
    Type type;


    openScope();
    params = new Parameters();
    params->_varargs = false;

    if (lookahead == VOID) {
	typespec = VOID;
	match(VOID);

	if (lookahead == ')')
	    return params;

    } else
	typespec = specifier();

    indirection = pointers();
    name = expect(ID);

    type = Type(typespec, indirection);
    declareVariable(name, type);
    params->_types.push_back(type);

    while (lookahead == ',') {
	match(',');

	if (lookahead == ELLIPSIS) {
	    params->_varargs = true;
	    match(ELLIPSIS);
	    break;
	}

	params->_types.push_back(parameter());
    }

    return params;
}


/*
 * Function:	globalDeclarator
 *
 * Description:	Parse a declarator, which in Simple C is either a scalar
 *		variable, an array, or a function, with optional pointer
 *		declarators.
 *
 *		global-declarator:
 *		  pointers identifier
 *		  pointers identifier [ num ]
 *		  pointers identifier ( parameters )
 */

static void globalDeclarator(int typespec)
{
    unsigned indirection;
    string name;


    indirection = pointers();
    name = expect(ID);

    if (lookahead == '[') {
	match('[');
	declareVariable(name, Type(typespec, indirection, number()));
	match(']');

    } else if (lookahead == '(') {
	match('(');
	declareFunction(name, Type(typespec, indirection, parameters()));
	closeScope();
	match(')');

    } else
	declareVariable(name, Type(typespec, indirection));
}


/*
 * Function:	remainingDeclarators
 *
 * Description:	Parse any remaining global declarators after the first.
 *
 * 		remaining-declarators
 * 		  ;
 * 		  , global-declarator remaining-declarators
 */

static void remainingDeclarators(int typespec)
{
    while (lookahead == ',') {
	match(',');
	globalDeclarator(typespec);
    }

    match(';');
}


/*
 * Function:	topLevelDeclaration
 *
 * Description:	Parse a global declaration or function definition.
 *
 * 		global-or-function:
 * 		  specifier pointers identifier remaining-decls
 * 		  specifier pointers identifier [ num ] remaining-decls
 * 		  specifier pointers identifier ( parameters ) remaining-decls 
 * 		  specifier pointers identifier ( parameters ) { ... }
 */

static void topLevelDeclaration()
{
    int typespec;
    unsigned indirection;
    Parameters *params;
    string name;


    typespec = specifier();
    indirection = pointers();
    name = expect(ID);

    if (lookahead == '[') {
	match('[');
	declareVariable(name, Type(typespec, indirection, number()));
	match(']');
	remainingDeclarators(typespec);

    } else if (lookahead == '(') {
	match('(');
	params = parameters();
	match(')');

	if (lookahead == '{') {
	    defineFunction(name, Type(typespec, indirection, params));
	    match('{');
	    declarations();
	    statements();
	    closeScope();
	    match('}');

	} else {
	    closeScope();
	    declareFunction(name, Type(typespec, indirection, params));
	    remainingDeclarators(typespec);
	}

    } else {
	declareVariable(name, Type(typespec, indirection));
	remainingDeclarators(typespec);
    }
}


/*
 * Function:	main
 *
 * Description:	Analyze the standard input stream.
 */

int main()
{
    openScope();
    lookahead = yylex();

    while (lookahead != DONE)
	topLevelDeclaration();

    closeScope();
    exit(EXIT_SUCCESS);
}
