%{
/*
 * File:	lexer.l
 *
 * Description:	This file contains the flex description for the lexical
 *		analyzer for Simple C.
 */

# include <string>
# include <iostream>
# include "lexer.h"
# include "tokens.h"

using namespace std;

int numerrors = 0;
static void ignoreComment();
%}

%option nounput noyywrap
%%

"/*"				{ignoreComment();}

"auto"				{return AUTO;}
"break"				{return BREAK;}
"case"				{return CASE;}
"char"				{return CHAR;}
"const"				{return CONST;}
"continue"			{return CONTINUE;}
"default"			{return DEFAULT;}
"do"				{return DO;}
"double"			{return DOUBLE;}
"else"				{return ELSE;}
"enum"				{return ENUM;}
"extern"			{return EXTERN;}
"float"				{return FLOAT;}
"for"				{return FOR;}
"goto"				{return GOTO;}
"if"				{return IF;}
"int"				{return INT;}
"long"				{return LONG;}
"register"			{return REGISTER;}
"return"			{return RETURN;}
"short"				{return SHORT;}
"signed"			{return SIGNED;}
"sizeof"			{return SIZEOF;}
"static"			{return STATIC;}
"struct"			{return STRUCT;}
"switch"			{return SWITCH;}
"typedef"			{return TYPEDEF;}
"union"				{return UNION;}
"unsigned"			{return UNSIGNED;}
"void"				{return VOID;}
"volatile"			{return VOLATILE;}
"while"				{return WHILE;}

"||"				{return OR;}
"&&"				{return AND;}
"=="				{return EQL;}
"!="				{return NEQ;}
"<="				{return LEQ;}
">="				{return GEQ;}
"++"				{return INC;}
"--"				{return DEC;}
"->"				{return ARROW;}
"..."				{return ELLIPSIS;}
[-=|<>+*/%&!()\[\]{};:.,]	{return *yytext;}

0|[1-9][0-9]*			{return NUM;}
0[xX][0-9a-fA-F]+		{return NUM;}
[a-zA-Z_][a-zA-Z_0-9]*		{return ID;}
\"(\\.|[^\\\n"])*\"		{return STRING;}
\'(\\.|[^\\\n'])*\'		{return CHARACTER;}

\n+				{yylineno += yyleng;}
[ \t]+				{/* ignored */}
.				{/* ignored */}

%%

/*
 * Function:	ignoreComment
 *
 * Description:	Ignore a comment after recognizing its beginning.
 */

static void ignoreComment()
{
    int c1, c2;


    while ((c1 = yyinput()) != 0) {
	while (c1 == '*') {
	    if ((c2 = yyinput()) == '/' || c2 == 0)
		return;

	    c1 = c2;
	}

	if (c1 == '\n')
	    yylineno ++;
    }

    if (c1 == 0)
	report("unterminated comment");
}


/*
 * Function:	report
 *
 * Description:	Report an error to the standard error prefixed with the
 *		line number.  We'll be using this a lot later with an
 *		optional string argument, but C++'s stupid streams don't do
 *		positional arguments, so we actually resort to snprintf.
 *		You just can't beat C for doing things down and dirty.
 */

void report(const string &str, const string &arg)
{
    char buf[1000];

    snprintf(buf, sizeof(buf), str.c_str(), arg.c_str());
    cerr << "line " << yylineno << ": " << buf << endl;
    numerrors ++;
}
