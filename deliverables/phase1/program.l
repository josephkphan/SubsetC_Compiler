
%{

/*
 * File:        program.l 
 * Maintainer:  Joseph Phan
 * Created On:  1/10/2017
 * Purpose:     Santa Clara University Assignment : Coen 175 Compilers Assignment : "Phase 1"
 * Description: This program uses flex to perform Lexical Analysis on an input C Program. It 
 *              will identify whether a token is a digit, hex, keyword, operator, string, or 
 *              charcter. It will ignore whitespace and comments. 
 *
 * Makefile:    How this works is that flex takes in a file as an input. With the given regex,
 *              functions, and main. It will generate a cpp program that acts as a compiler. 
 *              This cpp file takes a C program as stdin and will output the lexical Analysis
 *              results.
 */

#include <math.h>
/* need this for the call to atof() */

using namespace std;

%}

%option nounput noyywrap

WHITESPACE [ \t\n]+
COMMENT    "/*"
KEYWORD    auto|break|case|char|const|continue|default|do|double|else|enum|extern|float|for|goto|if|int|long|register|return|short|signed|sizeof|static|struct|switch|typedef|union|unsigned|void|volatile|while
STRING     \"(\\.|[^"\\])*\"
CHARACTER  '(\\.|[^'\\])+'
ID         [a-zA-Z_][a-zA-Z_0-9]*
DIGIT      0|[1-9][0-9]*
HEX        0[xX][0-9a-fA-F]+
OPERATOR   "="|"||"|"&&"|"|"|"=="|"!="|"<"|">"|"<="|">="|"+"|"-"|"*"|"/"|"%"|"&"|"!"|"++"|"--"|"->"|"("|")"|"["|"]"|"{"|"}"|";"|":"|"."|","|"..."
UNKNOWN    .

%%

{WHITESPACE} {   

}

{COMMENT} {
    char prev = yyinput();
    char cur = yyinput();
    while (!(prev == '*' && cur == '/') ){
        prev = cur;
        cur = yyinput();
    }
}

{KEYWORD} {
    printf( "keyword %s\n", yytext );
}

{STRING} {
    printf( "string %s\n", yytext );
}

{CHARACTER} {
    printf( "character %s\n", yytext );
}

{ID} {
    printf( "identifier %s\n", yytext );
}  

{DIGIT} {
    printf( "decimal %s\n", yytext,
    atoi( yytext ) );
}

{HEX} {
    printf( "hexadecimal %s\n", yytext,
    atoi( yytext ) );
}

{OPERATOR} {
    printf( "operator %s\n", yytext );
}

{UNKNOWN} {

}

%%

int main(){
    yylex();
    return 0;
}