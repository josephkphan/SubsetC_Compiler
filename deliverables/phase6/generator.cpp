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
static Label *return_label;
static vector<Label> loop_labels;
vector<string> stringLabels;

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

std::string IntToString (int number){
  std::ostringstream oss;
  oss<< number;
  return oss.str();
}

void generator_debug(string message){
    // cout << "--" << message << endl;
}

void assignTemp(Expression* expr){
    generator_debug("Expression::generate()");
    stringstream ss;
	//temp_offset -= 4;
	temp_offset -= expr->type().size();
    ss << temp_offset << "(%ebp)";
    expr->_operand = ss.str();
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

    if (_symbol->_offset != 0){
		ss << _symbol->_offset << "(%ebp)";
	}else{
		ss << global_prefix << _symbol->name();
	}
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
	assignTemp(this);

    for (int i = _args.size() - 1; i >= 0; i --) {
		_args[i]->generate();
		cout << "\tpushl\t" << _args[i] << endl;
		numBytes += _args[i]->type().size();
    }

    cout << "\tcall\t" << global_prefix << _id->name() << endl;

	cout << "\tmovl\t%eax, " << _operand << endl;
    if (numBytes > 0)
	cout << "\taddl\t$" << numBytes << ", %esp" << endl;

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
	if (_args.size() > maxargs)
		maxargs = _args.size();

	for (int i = _args.size() - 1; i >= 0; i --) {
		_args[i]->generate();
		cout << "\tmovl\t" << _args[i] << ", %eax" << endl;
		cout << "\tmovl\t%eax, " << i * SIZEOF_ARG << "(%esp)" << endl;
	}

	cout << "\tcall\t" << global_prefix << _id->name() << endl;
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

void Assignment::generate(){
	bool indirect;
    _right->generate();
    _left->generate(indirect);

    cout << "\tmovl\t" << _right << ", %eax" << endl;

	if(_left->type().size() == SIZEOF_INT){
		if(indirect){
			cout << "\tmovl\t" << _left << ", %ecx" << endl;
			cout << "\tmovl\t%eax, (%ecx)" << endl;
		}else {
			cout << "\tmovl\t%eax, " << _left << endl;
		}
	}else{
		if(indirect){
			cout << "\tmovl\t" << _left << ", %ecx" << endl;
			cout << "\tmovb\t%al, (%ecx)" << endl;
		}else {
			cout << "\tmovb\t%al, " << _left << endl;
		}
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

void Function::generate(){
	return_label = new Label();
    int offset = 0;

    // Prologue
	allocate(offset);
	generator_debug("PROLOGUE!!!");
    generator_debug("Function::generate() - 1");
    cout << global_prefix << _id->name() << ":" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << _id->name() << ".size, %esp" << endl;


   // Body
	generator_debug("BODY!!!");
    maxargs = 0;
	temp_offset = offset; 
    _body->generate();
	offset = temp_offset; 
    offset -= maxargs * SIZEOF_ARG;

    generator_debug("Function::generate() - 2");
    while ((offset - PARAM_OFFSET) % STACK_ALIGNMENT){
		offset --;
	}
	cout << *return_label << ":" << endl; 

    // Epilogue
	generator_debug("EPILOGUE!!!");
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

void generateGlobals(const Symbols &globals){
    if (globals.size() > 0){ 
		cout << "\t.data" << endl;
	}

    for (unsigned i = 0; i < globals.size(); i ++) {
		cout << "\t.comm\t" << global_prefix << globals[i]->name();
		cout << ", " << globals[i]->type().size();
		cout << ", " << globals[i]->type().alignment() << endl;
    }

	for (unsigned i = 0; i < stringLabels.size(); i++){
		cout << stringLabels[i] << endl;
	}

}

//-------------------------  Generate String --------------------------- //

void String::generate(){
	Label string_label; 
    stringstream ss_label;
	ss_label << string_label;
	stringLabels.push_back(ss_label.str() +":\t.asciz\t" + _value);
	_operand = ss_label.str();
}

//-------------------------  Expression Functions  --------------------------- //
void Expression::generate(bool &indirect){
    generator_debug("Expression::generate(bool &indirect)");
    indirect = false;
    generate();
}

void Expression::generate(){
    generator_debug("Expression::generate()");
    cerr << "have not finished assignment" << endl;
}

void Promote::generate(){
	_expr->generate();
	assignTemp(this);	

	generator_debug("Promote::generate()");
	
	cout << "\tmovb\t" << _expr << ", %al" << endl;
	cout << "\tmovsbl\t%al, %eax" << endl;
	cout << "\tmovl\t%eax, " << this << endl;

}

//------------ Expression - Binary Operators *, /, %, +, -  ------------- //

void Add::generate(){
	_left->generate();
	_right->generate();
	assignTemp(this);

    generator_debug("Add::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\taddl\t" << _right << ", %eax" << endl;
	cout << "\tmovl\t%eax,"<< _operand << endl;
}

void Subtract::generate(){
    generator_debug("Subtract::generate()");
	_left->generate();
	_right->generate();
	assignTemp(this);

	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tsubl\t" << _right << ", %eax" << endl;
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void Multiply::generate(){
	_left->generate();
	_right->generate();
	assignTemp(this);

    generator_debug("Multiply::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\timull\t" << _right << ", %eax" << endl;
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void Divide::generate(){
	_left->generate();
	_right->generate();
	assignTemp(this);

    generator_debug("Divide::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tmovl\t" << _right << ", %ecx" << endl;
	cout << "\tcltd\t" << endl;
	cout << "\tidivl\t%ecx" << endl;
	cout << "\tmovl\t%eax, " << _operand << endl; 
}

void Remainder::generate(){
	_left->generate();
	_right->generate();
	assignTemp(this);
	
    generator_debug("Remainder::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tmovl\t" << _right << ", %ecx" << endl;
	cout << "\tcltd\t" << endl;
	cout << "\tidivl\t%ecx" << endl;
	cout << "\tmovl\t%edx, " << _operand << endl; 
}

//------------ Expression - Uniary Operators   ------------- //
void Not::generate(){
	_expr->generate();
	assignTemp(this);

    generator_debug("Not::generate() - 2");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tsete\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

	cout << "\tmovl\t%eax, " << _operand << endl;
}

void Dereference::generate(){	
	_expr->generate();
	assignTemp(this);
	
    generator_debug("Dereference::generate()");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	
	if(_type.size() == SIZEOF_CHAR){
		cout << "\tmovsbl\t(%eax), %eax" << endl;
	}else {
		cout << "\tmovl\t(%eax), %eax" << endl;
	}
	
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void Address::generate(){
	bool indirect;
	_expr->generate(indirect);
	if(indirect){ 
		_operand = _expr->_operand;
	}else{
		assignTemp(this);
        generator_debug("Address::generate()");
		cout << "\tleal\t" << _expr << ", %eax" << endl;
		cout << "\tmovl\t%eax, " << _operand << endl;
	}
}

void Dereference::generate(bool &indirect){
    generator_debug("Dereference::generate()");
	indirect = true;
	_expr->generate(); 
	_operand = _expr->_operand; 
}

void Negate::generate(){
	_expr->generate();
    generator_debug("Negate::generate()");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tnegl\t%eax" << endl;
	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

//------------ Expression - Comparison Operators  >, >=, <, <=  ------------- //

void GreaterThan::generate(){
	_left->generate();
	_right->generate();

    generator_debug("GreaterThan::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetg\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;

	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void GreaterOrEqual::generate(){
	_left->generate();
	_right->generate();

    generator_debug("GreaterOrEqual::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetge\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void LessThan::generate(){
	_left->generate();
	_right->generate();

    generator_debug("LessThan::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetl\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void LessOrEqual::generate(){
    generator_debug("LessOrEqual::generate() - 1");
	_left->generate();
	_right->generate();

	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetle\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;

 	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

//------------ Expression - Equality Operators  =, != ------------- //
void NotEqual::generate(){
	_left->generate();
	_right->generate();

    generator_debug("NotEqual::generate()");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsetne\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;
	
	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

void Equal::generate(){
	_left->generate();
	_right->generate();

    generator_debug("Equal::generate() - 1");
	cout << "\tmovl\t" << _left << ", %eax" << endl;
	cout << "\tcmpl\t" << _right << ", %eax" << endl;
	cout << "\tsete\t%al" << endl;
	cout << "\tmovzbl\t%al, %eax" << endl;
	assignTemp(this);
	cout << "\tmovl\t%eax, " << _operand << endl;
}

//------------ Expression - Comparison Operators ||, && ------------- //
void LogicalAnd::generate(){
	_left->generate();
	Label and_label = Label();

    generator_debug("LogicalAnd::generate() - 1");
	cout << "movl\t" << _left << ",%eax" << endl;
	cout << "cmpl\t$0,%eax" << endl;
	cout << "je\t" << and_label << endl; 
	
	_right->generate();
    generator_debug("LogicalAnd::generate() - 2");
	cout << "movl\t" << _right << ",%eax" << endl;
	cout << "cmpl\t$0,%eax" << endl;
	cout << and_label << ":" << endl;
	assignTemp(this);
	cout << "\tsetne\t%al" << endl;
	cout << "\tmovzbl\t%al,%eax" << endl;
	cout << "\tmovl\t%eax," << _operand << endl;
}

void LogicalOr::generate(){
	_left->generate();
	Label or_label = Label();

	generator_debug("LogicalOr::generate() - 1");
	cout << "\tmovl\t" << _left << ",%eax" << endl;
	cout << "\tcmpl\t$0,%eax" << endl;
	cout << "\tjne\t" << or_label << endl;
	
	_right->generate();
    generator_debug("LogicalOr::generate() - 2");
	cout << "\tmovl\t" << _right << ",%eax" << endl;
	cout << "\tcmpl\t$0,%eax" << endl;

	cout << or_label << ":" << endl; 
    assignTemp(this);
    generator_debug("LogicalOr::generate() - 3");
	cout << "\tsetne\t%al" << endl;
	cout << "\tmovzbl\t%al,%eax" << endl;
	cout << "\tmovl\t%eax," << _operand << endl;
}

//------------ For Loops and While Loops ------------- //
void While::generate(){
	Label while_label, exit_while_label;
	loop_labels.push_back(exit_while_label);

	cout << while_label << ":" << endl;
	_expr->generate();
    generator_debug("While::generate() - 1");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tje\t\t" << exit_while_label << endl;

	_stmt->generate();
    generator_debug("While::generate() - 2");
	cout << "\tjmp\t\t" << while_label << endl;
	cout << exit_while_label << ":" << endl;
	loop_labels.pop_back();
}

void For::generate(){
	Label for_label, exit_for_label;
	loop_labels.push_back(exit_for_label);

	_init->generate();
	cout << for_label << ":" << endl;

	_expr->generate();
    generator_debug("For::generate() -1");
	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tje\t\t" << exit_for_label << endl;

	_stmt->generate();
	_incr->generate();
    generator_debug("For::generate() -2");
	cout << "\tjmp\t\t" << for_label << endl;
	cout << exit_for_label << ":" << endl;
	loop_labels.pop_back();
	
}
//------------ If Statements ------------- //

void If::generate(){
    generator_debug("If::generate()");
	_expr->generate();
	Label if_label, else_label;

	cout << "\tmovl\t" << _expr << ", %eax" << endl;
	cout << "\tcmpl\t$0, %eax" << endl;
	cout << "\tje\t\t" << if_label << endl;

	_thenStmt->generate();
	if( _elseStmt==NULL ){ 
		cout << if_label << ":" << endl;
	}else {
		cout << "\tjmp\t\t" << else_label << endl;
		cout << if_label << ":" << endl;
		_elseStmt->generate();
	}
	cout << else_label << ":" << endl;	
}

//------------ Break Statement ------------- //

void Break::generate(){	
	generator_debug("Break::generate()");
	cout << "\tjmp\t\t" << loop_labels.back() << endl;
    // cout << "IN BREAK!!!!!!" << endl;
}

//------------ Return  Statement ------------- //
void Return::generate(){
	_expr->generate();
	
    generator_debug("Return::generate()");
	cout << "\tmovl\t" << _expr << ", %eax" << endl; 
	cout << "\tjmp\t\t" << *return_label << endl;
}


