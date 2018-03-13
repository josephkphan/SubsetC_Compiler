/*
 * File:	generator.cpp
 *
 * Description:	This file contains the public and member function
 *		definitions for the code generator for Simple C.
 *
 *		Extra functionality:
 *		- putting all the global declarations at the end
 */

# include <sstream>
# include <iostream>
# include <string>
# include "generator.h"
# include "machine.h"
# include "label.h"
# include "Type.h"

using namespace std;

static unsigned maxargs;
int temp_offset;

static Label *retlbl;

std::vector<string> strs;

/*
 * Function:	operator <<
 *
 * Description:	Convenience function for writing the operand of an
 *		expression.
 */

ostream &operator <<(ostream &ostr, Expression *expr){
    return ostr << expr->_operand;
}

ostream &operator <<(ostream &ostr, const Label &lbl){
	return ostr << ".L" << lbl.number();
}

void generator_debug(string message){
    // cout << "--" << message << endl;
}

string getTemp()
{
    generator_debug("Expression::generate()");
    stringstream ss;
    temp_offset -= 4;
    ss << temp_offset << "(%ebp)";
    return ss.str();
}

/*
 * Function:	Identifier::generate
 *
 * Description:	Generate code for an identifier.  Since there is really no
 *		code to generate, we simply update our operand.
 */

void Identifier::generate(){
    generator_debug("Identifier::generate()");
    stringstream ss;


    if (_symbol->_offset != 0)
	ss << _symbol->_offset << "(%ebp)";
    else
	ss << global_prefix << _symbol->name();

    _operand = ss.str();
}


/*
 * Function:	Number::generate
 *
 * Description:	Generate code for a number.  Since there is really no code
 *		to generate, we simply update our operand.
 */

void Number::generate(){
    stringstream ss;


    ss << "$" << _value;
    _operand = ss.str();
}


# if STACK_ALIGNMENT == 4

/*
 * Function:	Call::generate
 *
 * Description:	Generate code for a function call expression, in which each
 *		argument is simply a variable or an integer literal.
 */

void Call::generate()
{
    unsigned numBytes = 0;
	_operand = getTemp();

    for (int i = _args.size() - 1; i >= 0; i --) {
		_args[i]->generate();
		cout << "\tpushl\t" << _args[i] << endl;
		numBytes += _args[i]->type().size();
    }

    cout << "\tcall\t" << global_prefix << _id->name() << endl;

    if (numBytes > 0)
	cout << "\taddl\t$" << numBytes << ", %esp" << endl;

	cout << "\tmovl\t%eax, " << _operand << endl;
}


# else

/*
 * If the stack has to be aligned to a certain size before a function call
 * then we cannot push the arguments in the order we see them.  If we had
 * nested function calls, we cannot guarantee that the stack would be
 * aligned.
 *
 * Instead, we must know the maximum number of arguments so we can compute
 * the size of the frame.  Again, we cannot just move the arguments onto
 * the stack as we see them because of nested function calls.  Rather, we
 * have to generate code for all arguments first and then move the results
 * onto the stack.  This will likely cause a lot of spills.
 *
 * For now, since each argument is going to be either a number of in
 * memory, we just load it into %eax and then move %eax onto the stack.
 */

void Call::generate()
{
    // unsigned numBytes = 0;
	// _operand = getTemp();

    // for (int i = _args.size() - 1; i >= 0; i --) {
	// 	_args[i]->generate();
	// 	cout << "\tpushl\t" << _args[i] << endl;
	// 	numBytes += _args[i]->type().size();
    // }

    // cout << "\tcall\t" << global_prefix << _id->name() << endl;

    // if (numBytes > 0)
	// cout << "\taddl\t$" << numBytes << ", %esp" << endl;

    // cout << "\tmovl\t%eax, " << _operand << endl;
    
    //     unsigned numBytes = 0;


    for (int i = _args.size() - 1; i >= 0; i --) {
	_args[i]->generate();
	cout << "\tpushl\t" << _args[i] << endl;
	numBytes += _args[i]->type().size();
    }

    cout << "\tcall\t" << global_prefix << _id->name() << endl;

    if (numBytes > 0)
    {
	cout << "\taddl\t$" << numBytes << ", %esp" << endl;
    }
    _operand = getTemp();
    cout << "\tmovl\t%eax, " << _operand << endl;
}

# endif


/*
 * Function:	Assignment::generate
 *
 * Description:	Generate code for this assignment statement, in which the
 *		right-hand side is an integer literal and the left-hand
 *		side is an integer scalar variable.  Actually, the way
 *		we've written things, the right-side can be a variable too.
 */

void Assignment::generate()
{
    // _left->generate();
    // _right->generate();

    // generator_debug("Assignment::generate()");
    // cout << "\tmovl\t" << _right << ", %eax" << endl;
    // cout << "\tmovl\t%eax, " << _left << endl;

    	bool indirect;
    _left->generate(indirect);
    _right->generate();

    cout << "\tmovl\t" << _right << ", %eax" << endl;

	if(_left->type().size() == SIZEOF_INT)
	{
		if(indirect)
		{
			cout << "\tmovl\t" << _left << ", %ecx" << endl;
			cout << "\tmovl\t%eax, (%ecx)" << endl;
		}
		else cout << "\tmovl\t%eax, " << _left << endl;
	}
	else
	{
		if(indirect)
		{
			cout << "\tmovl\t" << _left << ", %ecx" << endl;
			cout << "\tmovb\t%al, (%ecx)" << endl;
		}
		else cout << "\tmovb\t%al, " << _left << endl;
	}
}


/*
 * Function:	Block::generate
 *
 * Description:	Generate code for this block, which simply means we
 *		generate code for each statement within the block.
 */

void Block::generate()
{
    generator_debug("Block::generate()");
    for (unsigned i = 0; i < _stmts.size(); i ++)
	_stmts[i]->generate();
}


/*
 * Function:	Function::generate
 *
 * Description:	Generate code for this function, which entails allocating
 *		space for local variables, then emitting our prologue, the
 *		body of the function, and the epilogue.
 */

void Function::generate()
{
    int offset = 0;

	retlbl = new Label();

    /* Generate our prologue. */

    allocate(offset);
    generator_debug("Function::generate() - 1");
    cout << global_prefix << _id->name() << ":" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << _id->name() << ".size, %esp" << endl;


    /* Generate the body of this function. */

    maxargs = 0;
	temp_offset = offset; //Store offset in temp_offset
    _body->generate();
	offset = temp_offset; //Restore offset from temp_offset

    offset -= maxargs * SIZEOF_ARG;

    generator_debug("Function::generate() - 2");
    while ((offset - PARAM_OFFSET) % STACK_ALIGNMENT)
	offset --;

	cout << *retlbl << ":" << endl; 
    /* Generate our epilogue. */

    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl << endl;

    cout << "\t.globl\t" << global_prefix << _id->name() << endl;
    cout << "\t.set\t" << _id->name() << ".size, " << -offset << endl;

    cout << endl;
}


/*
 * Function:	generateGlobals
 *
 * Description:	Generate code for any global variable declarations.
 */

void generateGlobals(const Symbols &globals)
{
    generator_debug("generateGlobals()");
    if (globals.size() > 0)
	cout << "\t.data" << endl;

    for (unsigned i = 0; i < globals.size(); i ++) {
	cout << "\t.comm\t" << global_prefix << globals[i]->name();
	cout << ", " << globals[i]->type().size();
	cout << ", " << globals[i]->type().alignment() << endl;
    }
}




//-------------------------  New Stuff  ---------------------------//



void Expression::generate(bool &indirect)
{
    generator_debug("Expression::generate(bool &indirect)");
    indirect = false;
    generate();
}

void Expression::generate()
{
    generator_debug("Expression::generate()");
    cerr << "have not finished assignment" << endl;
}

void Dereference::generate()
{	
	_expr->generate();
	_operand = getTemp();
	
    generator_debug("Dereference::generate()");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	
	if(_type.size() == SIZEOF_CHAR) cout << "\tmovsbl\t(%eax), %eax" << endl;
	else 							cout << "\tmovl\t(%eax), %eax" << endl;
	
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void Dereference::generate(bool &indirect)
{
    generator_debug("Dereference::generate()");
	indirect = true; //Set indirect equal to true, since we're now dealing with a pointer
	_expr->generate(); //Generate code for the sub-expression
	_operand = _expr->_operand; 
}

/*
 * Function: Address::generate
 *
 * Description: Generate code for the address operator, and etc. recursively
 */
void Address::generate()
{
	bool indirect;
	_expr->generate(indirect);
	if(indirect) _operand = _expr->_operand;
	else
	{
		_operand = getTemp();
        generator_debug("Address::generate()");
		cout << "\tleal\t" << _expr << ", %eax" << endl;
		cout << "\tmovl\t%eax, " << _operand << endl;
	}
}

/*
 * Function: Multiply::generate
 *
 * Description: Generate code for the multiplication operator, and etc. recursively
 */

void Multiply::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("Multiply::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\timull\t" << _right << ", %eax" << endl;
	
	_operand = getTemp();

	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: Divide::generate
 *
 * Description: Generate code for the division operator, and etc. recursively
 */

void Divide::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("Divide::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tmovl\t" << _right << ", %ecx" << endl;
	cout << "\tcltd\t" << endl;
	cout << "\tidivl\t%ecx" << endl;

	_operand = getTemp();

	cout << "\tmovl\t%eax, " << _operand << endl; //%eax contains quotient
}

/*
 * Function: Remainder::generate
 *
 * Description: Generate code for the remainder operator, and etc. recursively
 */

void Remainder::generate()
{
	_left->generate();
	_right->generate();
	
    generator_debug("Remainder::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tmovl\t" << _right << ", %ecx" << endl;
	cout << "\tcltd\t" << endl;
	cout << "\tidivl\t%ecx" << endl;

	_operand = getTemp();

	cout << "\tmovl\t%edx, " << _operand << endl; //%edx contains remainder
}

/*
 * Function: Add::generate
 *
 * Description: Generate code for addition operator, and etc. recursively
 */

void Add::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("Add::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\taddl\t" << _right << ", %eax" << endl;
	
	_operand = getTemp();
	
	cout << "\tmovl\t%eax,"<< _operand << endl;
}

/*
 * Function: Subtract::generate
 *
 * Description: Generate code for subtraction operator, and etc. recursively
 */

void Subtract::generate()
{
    generator_debug("Subtract::generate()");
	_left->generate();
	_right->generate();

	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tsubl\t" << _right << ", %eax" << endl;

	_operand = getTemp();

	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: Negate::generate
 *
 * Description: Generate code for negation operator, and etc. recursively
 */

void Negate::generate()
{
	_expr->generate();
	
    generator_debug("Negate::generate()");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tnegl\t%eax" << endl;

	_operand = getTemp();

	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: Not::generate
 *
 * Description: Generate code for not operator, and etc. recursively
 */

void Not::generate()
{
	_expr->generate();

    generator_debug("Not::generate() - 2");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tsete\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: LessThan::generate
 *
 * Description: Generate code for less-than operator, and etc. recursively
 */

void LessThan::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("LessThan::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetl\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: GreaterThan::generate
 *
 * Description: Generate code for greater-than operator, and etc. recursively
 */

void GreaterThan::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("GreaterThan::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetg\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;

	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: LessOrEqual::generate
 *
 * Description: Generate code for less-than-or-equal operator, and etc. recursively
 */

void LessOrEqual::generate()
{
    generator_debug("LessOrEqual::generate() - 1");
	_left->generate();
	_right->generate();

	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetle\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

 	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: GreaterOrEqual::generate
 *
 * Description: Generate code for greater-than-or-equal operator, and etc. recursively
 */

void GreaterOrEqual::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("GreaterOrEqual::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetge\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: Equal::generate
 *
 * Description: Generate code for equality operator, and etc. recursively
 */

void Equal::generate()
{
	_left->generate();
	_right->generate();

    generator_debug("Equal::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsete\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;
	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: Equal::generate
 *
 * Description: Generate code for equality operator, and etc. recursively
 */

void NotEqual::generate()
{
	_left->generate();
	_right->generate();


    generator_debug("NotEqual::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetne\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;
	
	_operand = getTemp();
	cout << "\tmovl\t%eax, " << _operand << endl;
}

/*
 * Function: LogicalOr::generate
 *
 * Description: Generate code for logical-or operator, and etc. recursively
 */

void LogicalOr::generate()
{
	_left->generate();

	Label orlbl = Label();
	generator_debug("LogicalOr::generate() - 1");
	cout << "\tmovl\t" << _left << ",%eax" << endl;
	cout << "\tcmpl\t$0,%eax" << endl;
	cout << "\tjne\t" << orlbl << endl;
	
	_right->generate();
    generator_debug("LogicalOr::generate() - 2");
	cout << "\tmovl\t" << _right << ",%eax" << endl;
	cout << "\tcmpl\t$0,%eax" << endl;

	
	cout << orlbl << ":" << endl; 
    _operand = getTemp();
    generator_debug("LogicalOr::generate() - 3");
	cout << "\tsetne\t%al" << endl;
	cout << "\tmovzbl\t%al,%eax" << endl;
	cout << "\tmovl\t%eax," << _operand << endl;
	
}

/*
 * Function: LogicalAnd::generate
 *
 * Description: Generate code for logical-and operator, and etc. recursively
 */

void LogicalAnd::generate()
{
	_left->generate();

	Label andlbl = Label();

    generator_debug("LogicalAnd::generate() - 1");
	cout << "movl\t" << _left << ",%eax" << endl;
	cout << "cmpl\t$0,%eax" << endl;
	cout << "jne\t" << andlbl << endl; 
	
	_right->generate();
    generator_debug("LogicalAnd::generate() - 2");
	cout << "movl\t" << _right << ",%eax" << endl;
	cout << "cmpl\t$0,%eax" << endl;
	
	
	cout << andlbl << ":" << endl;
	_operand = getTemp();
	cout << "\tsetne\t%al" << endl;
	cout << "\tmovzbl\t%al,%eax" << endl;
	cout << "\tmovl\t%eax," << _operand << endl;

}

/*
 * Function: Return::generate
 *
 * Description: Generate code for return statements, and etc. recursively
 */
void Return::generate(){
	_expr->generate();
	
    generator_debug("Return::generate()");
	cout << "\tmovl\t" << _expr << ", %eax" << endl; //Store return value in %eax
	cout << "\tjmp\t\t" << *retlbl << endl; //Jump to return label
}

/*
 * Function: Promote::generate
 *
 * Description: Generate code for promotions
 */
void Promote::generate(){
	_expr->generate();
	_operand = getTemp();	
    generator_debug("Promote::generate()");
	//cout << "\tmovsbl\t" << _expr << ", " << _operand << endl; //One-line version
	cout << "\tmovsbl\t" << _expr << ", %eax" << endl; //Sign-extend expression
	cout << "\tmovl\t%eax, " << _operand << endl; //Store result in temp
}

/*
 * Function: While::generate
 *
 * Description: Generate code for while statements, and etc. recursively
 */
void While::generate(){
	//Loop label
	Label whileLbl;
	cout << whileLbl << ":" << endl;

	//Expression code gen
	_expr->generate();

    
	//Testing and expression
	Label exitLbl;
    generator_debug("While::generate() - 1");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tje\t\t" << exitLbl << endl;

	//Statement code gen
	_stmt->generate();

    //Either repeat the loop or exit
    generator_debug("While::generate() - 2");
	cout << "\tjmp\t\t" << whileLbl << endl;
	cout << exitLbl << ":" << endl;
}

/*
 * Function: For::generate
 *
 * Description: Generate code for for statements, and etc. recursively
 */
void For::generate(){
	//Initial statement code gen
	_init->generate();
	
	//Loop label
	Label forLbl;
	cout << forLbl << ":" << endl;

	//Expression code gen
	_expr->generate();

	//Testing expression
	Label exitLbl;
    generator_debug("For::generate() -1");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tje\t\t" << exitLbl << endl;

	//Loop statement code gen
	_stmt->generate();

	//Increment statement code gen
	_incr->generate();

    generator_debug("For::generate() -2");
	//Either repeat the loop or exit
	cout << "\tjmp\t\t" << forLbl << endl;
	cout << exitLbl << ":" << endl;
}

/*
 * Function: If::generate
 *
 * Description: Generate code for if statements, and etc. recursively
 */
void If::generate(){
    generator_debug("If::generate()");
	//Expression code gen
	_expr->generate();

	//If label
	Label ifLbl;

	//Testing expression
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tje\t\t" << ifLbl << endl;

	_thenStmt->generate();
	
	//Exit label
	Label exitLbl;
	
	//Check for absence of else statement
	if( _elseStmt==NULL ) cout << ifLbl << ":" << endl;
	
	else 
	{
		cout << "\tjmp\t\t" << exitLbl << endl;
		cout << ifLbl << ":" << endl;
		_elseStmt->generate();
	}

	cout << exitLbl << ":" << endl;	
}


void Break::generate(){	
    // generator_debug("Break::generate()");
    // cout << "IN BREAK!!!!!!" << endl;
}

void Break::allocate(int &offset) const {
//     generator_debug("Break::allocate(int &offset)");
//     cout << "IN BREAK!!!!!!" << endl;
}


void String::generate(){
    /*
    print this: .l3:    .asciz "%d\n"
    in use:     leal    .l3, %eax
    */

    // generator_debug("String::generate()");
    // cout << "#STRING" << endl;
    // stringstream ss;
    // Label *lbl = new Label();
    // ss << *lbl;
    // _operand = ss.str();
    // ss << ":\t.asciz " << _value << endl;
    // strs.push_back(ss.str());   

    // stringstream lblstr, out;
	
	// Label stringlbl; //Does this increment the counter?
	// lblstr << stringlbl;

	// out << lblstr.str() << ":\t.asciz\t" << _value;
	
	// stringLabels.push_back(out.str());

	// _operand = lblstr.str();
}
