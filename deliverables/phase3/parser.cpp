/*
 * File:	parser.c
 *
 * Description:	This file contains the public and private function and
 *		variable definitions for the recursive-descent parser for
 *		Simple C.
 */

# include <cstdlib>
# include <iostream>
# include "tokens.h"
# include "lexer.h"
# include "checker.h"

using namespace std;

static int lookahead;
static void expression();
static void statement();


/*
 * Function:	error
 *
 * Description:	Report a syntax error to standard error.
 */

static void error(){
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

static void match(int t){
    if (lookahead != t)
	error();

    lookahead = yylex();
}

static string match_and_save(int t){
    string buf = yytext;
    match(t);
    return buf;
}

static unsigned match_and_save_number(){
    return strtoul(match_and_save(NUM).c_str(), NULL, 0);
}

/*
 * Function:	isSpecifier
 *
 * Description:	Return whether the given token is a type specifier.
 */

static bool isSpecifier(int token){
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

static int specifier(){
	int typespec;
    if (isSpecifier(lookahead)){
		typespec = lookahead;
		match(lookahead);
	}else{
		typespec = ERROR;
		error();
	}
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

static unsigned pointers(){
	unsigned counter = 0;

    while (lookahead == '*'){
		match('*');
		counter++;
	}

	return counter;

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

static void declarator(int typespec){
	unsigned indirection = pointers();
	string name = match_and_save(ID);

    if (lookahead == '[') {
		match('[');
		declareVariable(name, Type(typespec, indirection, match_and_save_number()));
		match(']');
    }else {
		declareVariable(name, Type(typespec, indirection));
	}
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

static void declaration(){
	int typespec = specifier();
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

static void declarations(){
    while (isSpecifier(lookahead)){
		declaration();
	}
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

static void primaryExpression(){
	Type type;
	string name;
	int num;

    if (lookahead == '(') {
		match('(');
		expression();
		match(')');

    } else if (lookahead == CHARACTER) {
		name = match_and_save(CHARACTER);
    } else if (lookahead == STRING) {
		name = match_and_save(STRING);
    } else if (lookahead == NUM) {
		num = match_and_save_number();
    } else if (lookahead == ID) {
		string name = match_and_save(ID);
		Parameters *params = new Parameters;
	if (lookahead == '(') {
	    match('(');
		Type type(INT);
	    if (lookahead != ')') {
			expression();
			useSymbol(name, type);

			while (lookahead == ',') {
				match(',');
				expression();
			}
	    }

	    match(')');
	}
	useSymbol(name, type); 
    } else{
		error();
	}
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

static void postfixExpression(){
    primaryExpression();

    while (lookahead == '[') {
	match('[');
	expression();
	match(']');
    }
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

static void prefixExpression(){
    if (lookahead == '!') {
		match('!');
		prefixExpression();

    } else if (lookahead == '-') {
		match('-');
		prefixExpression();

    } else if (lookahead == '*') {
		match('*');
		prefixExpression();

    } else if (lookahead == '&') {
		match('&');
		prefixExpression();

    } else if (lookahead == SIZEOF) {
		match(SIZEOF);
		prefixExpression();

    } else{
		postfixExpression();
	}
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

static void multiplicativeExpression(){
    prefixExpression();

    while (1) {
		if (lookahead == '*') {
			match('*');
			prefixExpression();

		} else if (lookahead == '/') {
			match('/');
			prefixExpression();

		} else if (lookahead == '%') {
			match('%');
			prefixExpression();

		} else{
			break;
		}
    }
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

static void additiveExpression(){
    multiplicativeExpression();

    while (1) {
	if (lookahead == '+') {
	    match('+');
	    multiplicativeExpression();

	} else if (lookahead == '-') {
	    match('-');
	    multiplicativeExpression();

	} else
	    break;
    }
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

static void relationalExpression(){
    additiveExpression();

    while (1) {
	if (lookahead == '<') {
	    match('<');
	    additiveExpression();

	} else if (lookahead == '>') {
	    match('>');
	    additiveExpression();

	} else if (lookahead == LEQ) {
	    match(LEQ);
	    additiveExpression();

	} else if (lookahead == GEQ) {
	    match(GEQ);
	    additiveExpression();
	} else
	    break;
    }
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

static void equalityExpression(){
    relationalExpression();

    while (1) {
	if (lookahead == EQL) {
	    match(EQL);
	    relationalExpression();

	} else if (lookahead == NEQ) {
	    match(NEQ);
	    relationalExpression();

	} else
	    break;
    }
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

static void logicalAndExpression(){
    equalityExpression();

    while (lookahead == AND) {
	match(AND);
	equalityExpression();
    }
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

static void expression(){
    logicalAndExpression();

    while (lookahead == OR) {
		match(OR);
		logicalAndExpression();
    }
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

static void statements(){
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

static void assignment(){
    expression();

    if (lookahead == '=') {
		match('=');
		expression();
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
    if (lookahead == '{') {
		openScope(" statement level");
		match('{');
		declarations();
		statements();
		closeScope(" statement level");
		match('}');

    } else if (lookahead == BREAK) {
		match(BREAK);
		match(';');

    } else if (lookahead == RETURN) {
		match(RETURN);
		expression();
		match(';');

    } else if (lookahead == WHILE) {
		match(WHILE);
		match('(');
		expression();
		match(')');
		statement();

    } else if (lookahead == FOR) {
		match(FOR);
		match('(');
		assignment();
		match(';');
		expression();
		match(';');
		assignment();
		match(')');
		statement();

    } else if (lookahead == IF) {
		match(IF);
		match('(');
		expression();
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

static Type parameter(){
    int typespec = specifier();
    unsigned indirection = pointers();
	string name = match_and_save(ID);
	Type type(typespec, indirection);
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

static Parameters *parameters(){
	openScope("Inside Parameters");
    Parameters *params = new Parameters();

	int typespec;
    if (lookahead == VOID) {
		typespec = VOID;
		match(VOID);

		if (lookahead == ')'){ 
			return params;
		}

    } else{
		typespec = specifier();
	}
 
	unsigned indirection = pointers();
	string name = match_and_save(ID);
	
	Type type = Type(typespec, indirection);
    declareVariable(name, type);
    params->push_back(type);

    while (lookahead == ',') {
		match(',');
		if (lookahead == ELLIPSIS) {			//TODO should this still be here?
			match(ELLIPSIS);
			break;
		}
		params->push_back(parameter());
		
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

static void globalDeclarator(int typespec){
    unsigned indirection = pointers();
    string name = match_and_save(ID);

    if (lookahead == '[') {
		match('[');
		declareVariable(name, Type(typespec, indirection, match_and_save_number()));
		match(']');

    } else if (lookahead == '(') {
		match('(');
		openScope(" global scope");
		declareFunction(name, Type(typespec, indirection, parameters()));
		closeScope(" global scope");
		match(')');
    }else {
		declareVariable(name, Type(typespec, indirection));
	}
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
    int typespec = specifier();
    unsigned indirection = pointers();
    string name = match_and_save(ID);

    if (lookahead == '[') {
		match('[');
		declareVariable(name, Type(typespec, indirection, match_and_save_number()));
		match(']');
		remainingDeclarators(typespec);

    } else if (lookahead == '(') {
		match('(');
		// openScope(" top level");
		Parameters *params = parameters();
		match(')');

		if (lookahead == '{') {
			// defineFunction(name, Type(typespec, indirection, params));
			int ghetto_line_number = yylineno;
			match('{');
			declarations();
			statements();
			closeScope(" top level");
			match('}');
			defineFunction(name, Type(typespec, indirection, params), ghetto_line_number); 

		} else{
			closeScope(" top level");
			declareFunction(name, Type(typespec, indirection, params));
			remainingDeclarators(typespec);
		}

	} else{
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
    lookahead = yylex();
	openScope(" global level");
    while (lookahead != DONE)
	topLevelDeclaration();
	closeScope(" global level");
    exit(EXIT_SUCCESS);
}
