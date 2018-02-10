// ------------------------- Start C Program --------------------- //
# include <cctype>
# include <iostream>
# include "tokens.h"
# include "lexer.h"

using namespace std;

// ----------------------- Function Declarations ----------------- //
void expr();
void match(int t);
void error(string message);
void log(string message);
void debug_log(string message);
void exp_or();
void exp_and();
void exp_eqs();
void exp_compare();
void exp_as();
void exp_mdr();
void exp_unary();
void exp_index();
void terminals();
void ptrs();
void declarator();
void expression();
void expression_list();

void translation_unit();
void global_declarator_list();
void global_declarator();
void pointers();
void specifier();
void parameters();
void remaining_parameters();
void declarations();
void declarator_list();
void declarator();
void statements();
void statement();
void assignment();

// --------------------------- Program ------------------------- //
int lookahead; //used to peek at the next token 


int main()
{
    lookahead = yylex(); 
    translation_unit();
    return 0;
}

/**
 *  Matches next token against the input token. Moves lookahead variable
 */
void match( int token ) {
    if (token == lookahead){
        lookahead=yylex();  	//setup for the next function
    } else {
        error(" matching token ");
    }
}

/** 
 * Prints errors to stderr 
 */
void error(string message){
    cerr << "*****ERROR: " << message << endl;
}

/**
 * logger function used to cout to standard in (for output file)
 */
void log(string message){
    cout << message << endl;
}

/**
 * debug log
 */
void debug_log(string message){
    // cout << "*****DEBUG: " <<message << endl;
}



/**
 * Handles Or || 
 */
void exp_or(){
    debug_log("Inside OR");
    exp_and();
    while (lookahead == OR){
        match(OR);
        exp_and();
        log("or");
    }
    debug_log("Exiting OR"); 
}

/**
 * Handles And &&
 */
void exp_and(){
    debug_log("Inside AND");
    exp_eqs();
    while (lookahead == AND){
        match(AND);
        exp_eqs();
        log("and");
    }
    debug_log("Exiting AND"); 
}

/**
 * Handles Equality ==, !=   
 */
void exp_eqs(){
    debug_log("Inside EQ");
    exp_compare();
    while (true) {
        if (lookahead == EQUALEQUAL){
            match(EQUALEQUAL);
            exp_compare();
            log("eql");
        }else if (lookahead == NOT_EQUAL){
            match(NOT_EQUAL);
            exp_compare();
            log("neq");
        }else{
            break;
        }
    }
    debug_log("Exiting EQ"); 
}

/**
 * Handles Comparison Operators >, <, <=, >=
 */
void exp_compare(){
    debug_log("Inside CMP"); 
    exp_as();
    while (true) {
        if (lookahead == LESS){
            match(LESS);
            exp_as();
            log("ltn");
        }else if (lookahead == GREATER){
            match(GREATER);
            exp_as();
            log("gtn");
        }else if (lookahead == LESS_THAN){
            match(LESS_THAN);
            exp_as();
            log("leq");
        }else if (lookahead == GREATER_THAN){
            match(GREATER_THAN);
            exp_as();
            log("geq");
        }else{
            break;
        }
    }
    debug_log("Exiting CMP"); 
}

/**
 * Handles Addition, Subtraction Operators +, -
 */
void exp_as(){
    debug_log("Inside AS"); 
    exp_mdr();
    while (true) {
        if (lookahead == ADD){
            match(ADD);
            exp_mdr();
            log("add");
        }else if (lookahead == MINUS){
            match(MINUS);
            exp_mdr();
            log("sub");
        }else{
            break;
        }
    }
    debug_log("Exiting AS"); 
}

/**
 * Handles Multiplication, Division, Remainder Operators *, /, %
 */
void exp_mdr(){
    debug_log("Inside MDR"); 
    exp_unary();
    while (true) {
        if (lookahead == MULT){
            match(MULT);
            exp_unary();
            log("mul");
        }else if (lookahead == DIVIDE){
            match(DIVIDE);
            exp_unary();
            log("div");
        }else if (lookahead == PERCENT){
            match(PERCENT);
            exp_unary();
            log("rem");
        }else {
            break;
        }
    }
    debug_log("Exiting MDR");  
}

/**
 * Handles Unary Operators !, *, -, sizeof
 */
void exp_unary(){
    debug_log("Inside Unary"); 
    if (lookahead == NOT){
        match(NOT);
        exp_unary();
        log("not");
    }else if (lookahead == MULT){
        match(MULT);
        exp_unary();
        log("deref");
    }else if (lookahead == ADDRESS){
        match(ADDRESS);
        exp_unary();
        log("addr");
    }else if (lookahead == MINUS){
        match(MINUS);
        exp_unary();
        log("neg");
    }else if (lookahead == SIZEOF){
        match(SIZEOF);
        exp_unary();
        log("sizeof");
    }else {
        exp_index();
    }
    debug_log("Exiting Unary");  
}

/**
 *  Handles Array Index a[]
 */
void exp_index(){
    debug_log("Inside Index");  
    terminals();    
    while (lookahead == OPENBRACKET){
        match(OPENBRACKET);
        exp_or();
        match(CLOSEBRACKET);
        log("index");
    }
    debug_log("Exiting Index");  
}

/**
 * Handles terminal characters: id(), id, num, string, character, expression
 */ 
void terminals(){
    debug_log("Inside Terminals");
    if (lookahead == STRING){
        match(STRING);
        debug_log("Terminals - String");
    }else if (lookahead == CHARACTER){
        match(CHARACTER);
        debug_log("Terminals - Character");
    }else if (lookahead == NUM){
        match(NUM);
        debug_log("Terminals - Num");
    }else if (lookahead == ID){
        match(ID);
        debug_log("Terminals - ID");
        if (lookahead == OPENPAREN){
            match(OPENPAREN);
            if(lookahead == CLOSEPAREN){
                match(CLOSEPAREN);
            }else{
                expression_list();
                match(CLOSEPAREN);
            }
        }
    }else if (lookahead == OPENPAREN){
        debug_log("Terminals - Parenthesis");
        match(OPENPAREN);
        exp_or();
        match(CLOSEPAREN);
    }
    debug_log("Exiting Terminals");  
}

//TODO: ALSO DON'T FORGET ABOUT ID() and ID(expression_list)


/**--------------- NON Expression Functions ----------- **/

/**
 * translation-unit, Global Declaration, and Function Declaration
 */
void translation_unit(){
    debug_log("Inside translation_unit"); 
    while(lookahead == INT || lookahead == CHAR || lookahead == VOID){
        specifier();
        pointers();
        match(ID);
        if( lookahead == SEMICOLON){
            match(SEMICOLON);
        }else if (lookahead == OPENBRACKET){
            match(OPENBRACKET);
            match(NUM);
            match(CLOSEBRACKET);
            if(lookahead == SEMICOLON){
                match(SEMICOLON);
            }else{
                match(COMMA);
                global_declarator_list();
                match(SEMICOLON);
            }
        }else if (lookahead == COMMA){
            match(COMMA);
            global_declarator_list();
            match(SEMICOLON);
        }else if (lookahead == OPENPAREN){
            match(OPENPAREN);
            parameters();
            match(CLOSEPAREN);
            if (lookahead == SEMICOLON){
                match(SEMICOLON);
            }else if (lookahead == COMMA){
                match(COMMA);
                global_declarator_list();
                match(SEMICOLON);
            }else{
                match(OPENBRACE);
                declarations();
                statements();
                match(CLOSEBRACE);
            }
        }
    }
    debug_log("exiting translation_unit"); 
}


/**
 * global_declarator_list
 */
void global_declarator_list(){
    debug_log("Inside global_declarator_list"); 
    global_declarator();
    if (lookahead == COMMA){
        global_declarator_list();
    }
    debug_log("exiting global_declarator_list"); 
}

/**
 * global-declarator
 */
void global_declarator(){
    debug_log("Inside global_declarator"); 
    pointers();
    match(ID);
    if(lookahead == OPENPAREN){
        match(OPENPAREN);
        parameters();
        match(CLOSEPAREN);
    }else if (lookahead == OPENBRACKET){
        match(OPENBRACKET);
        match(NUM);
        match(CLOSEBRACKET);
    }
    debug_log("exiting global_declarator"); 
}


/**
 * pointers
 */
void pointers() {
    debug_log("Inside pointers"); 
    if (lookahead == MULT) {
        match(MULT);
        pointers();
    }
    debug_log("exiting pointers"); 
}

/**
 * specifier
 */
void specifier(){
    debug_log("Inside specifier"); 
    if (lookahead == INT) {
        match(INT);
    }else if (lookahead == CHAR) {
        match(CHAR);
    }else if (lookahead == VOID) {
        match(VOID);
    }
    debug_log("exiting specifier"); 
}

/**
 * parameters, parameter-list, parameter
 */
void parameters(){
    debug_log("Inside parameter"); 
    if (lookahead == VOID){
        match(VOID);
        if (lookahead == MULT || lookahead == ID){
            pointers();
            match(ID);
            remaining_parameters();
        }
    }else if(lookahead == CHAR){
        match(CHAR);
        pointers();
        match(ID);
        remaining_parameters();
    }else if(lookahead == INT){
        match(INT);
        pointers();
        match(ID);
        remaining_parameters();
    }
    debug_log("exiting parameter"); 
}

/**
 * after left recursion on Commas
 */
void remaining_parameters(){
    debug_log("Inside remaining_parameters"); 
    if(lookahead == COMMA){
        match(COMMA);
        if (lookahead == ELLIPSIS){
            match(ELLIPSIS);
        }else {
            parameters();
            remaining_parameters();
        }
    }
    debug_log("exiting remaining_parameters"); 
}

/**
 * declarations, declaration
 */
void declarations(){
    debug_log("Inside declarations");
    if(lookahead == INT || lookahead == CHAR || lookahead == VOID){ // Check for Null Case
        specifier();
        declarator_list();
        match(SEMICOLON);
        declarations();
    } 
    debug_log("exiting declarations"); 
}


/**
 * declarator_list
 */
void declarator_list(){
    debug_log("Inside declarator_list"); 
    declarator();
    if(lookahead == COMMA){
        match(COMMA);
        declarator_list();
    }
    debug_log("exiting declarator_list"); 
}

/**
 * declarator
 */
void declarator(){
    debug_log("Inside declarator"); 
    pointers();
    match(ID);
    if (lookahead == OPENBRACKET) {
        match(OPENBRACKET);
        match(NUM);
        match(CLOSEBRACKET);
    }  
    debug_log("exiting declarator"); 
}

/**
 * statements
 */
void statements(){
    debug_log("Inside statements"); 
    while(lookahead != CLOSEBRACE){
        statement();
    }
    debug_log("exiting statements"); 
}

/**
 * statement
 */
void statement(){
    debug_log("Inside statement"); 
    if(lookahead == OPENBRACE){
        match(OPENBRACE);
        declarations();
        statements();
        if(lookahead == CLOSEBRACE){
            match(CLOSEBRACE);
        }
    }else if(lookahead == BREAK){
        match(BREAK);
        match(SEMICOLON);
    }else if (lookahead == RETURN){
        match(RETURN);
        exp_or();
        match(SEMICOLON);
    }else if (lookahead == WHILE){
        match(WHILE);
        match(OPENPAREN);
        exp_or();
        match(CLOSEPAREN);
        statement();
    }else if (lookahead == FOR){
        match(FOR);
        match(OPENPAREN);
        assignment();
        match(SEMICOLON);
        exp_or();
        match(SEMICOLON);
        assignment();
        match(CLOSEPAREN);
        statement();
    }else if (lookahead == IF){
        match(IF);
        match(OPENPAREN);
        exp_or();
        match(CLOSEPAREN);
        statement();
        if(lookahead == ELSE){
            statement();
        }
    }else{
        assignment();
        match(SEMICOLON);
    }
    debug_log("exiting statement"); 
}

/**
 * assignment
 */
void assignment(){
    debug_log("Inside assignment"); 
    exp_or();
    if (lookahead == EQUAL){
        match(EQUAL);
        exp_or();
    }
    debug_log("exiting assignment"); 
}

/**
 * expression_list
 */
void expression_list(){
    exp_or();
    if(lookahead == COMMA){
        match(COMMA);
        expression_list();
    }
}