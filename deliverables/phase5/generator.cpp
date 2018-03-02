# include <sstream>
# include <iostream>
# include "Symbol.h"
# include "generator.h"

using namespace std;

static unsigned maxargs;


ostream &operator <<(ostream &ostr, Expression *expr)
{
    return ostr << expr->_operand;
}

void Identifier::generate()
{
    stringstream ss;


    if (_symbol->_offset != 0)
	ss << _symbol->_offset << "(%ebp)";
    else
	ss << _symbol->name();

    _operand = ss.str();
}


void Number::generate()
{
    stringstream ss;


    ss << "$" << _value;
    _operand = ss.str();
}

void Call::generate()
{
    unsigned numBytes = 0;


    for (int i = _args.size() - 1; i >= 0; i --) {
        _args[i]->generate();
        // cout << _args[i]
        if (_args[i]->type().isInteger()){
            cout << "\tpushl\t" << _args[i]->_operand << endl;

        }else {
            cout << "\tpushl\t-" << numBytes + _args[i]->type().size() << "(%ebp)" << endl;

        }
        numBytes += _args[i]->type().size();
    }

    cout << "\tcall\t"  << _id->name() << endl;

    if (numBytes > 0)
	cout << "\taddl\t$" << numBytes << ", %esp" << endl;
}


void Assignment::generate()
{
    _left->generate();
    _right->generate();

    cout << "\tmovl\t" << _right << ", %eax" << endl;
    cout << "\tmovl\t%eax, " << _left << endl;
}

void Block::generate()
{
    for (unsigned i = 0; i < _stmts.size(); i ++)
	_stmts[i]->generate();
}

void Function::allocate(int &offset)  {
Symbols syms = _body->declarations()->symbols();
int paramCount = _id->type().parameters()->_types.size();
int localVarCount = syms.size();
int paramOffset = 8;
int localVarOffset = 0;

for (int i = 0; i < paramCount; i++) {
// cout << "param offset: " << paramOffset << endl;
syms[i]->_offset = paramOffset;
paramOffset += 4;
}

for (int i = paramCount; i < localVarCount; i++) {
int multiply = 1;

if (syms[i]->type().isArray()) {
multiply = syms[i]->type().length();
}

localVarOffset -= 4 * multiply;
syms[i]->_offset = localVarOffset;
}
offset = localVarOffset;
// cout << "final offset: " << offset << endl;

}
void Function::generate()
{
    int offset = 0;

    
    /* Generate our prologue. */

    allocate(offset);

    cout << "\t.globl\t" << _id->name() << endl;
    
    cout << _id->name() << ":" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << -offset << ", %esp" << endl;


    /* Generate the body of this function. */

    maxargs = 0;
    _body->generate();

    offset -= maxargs * 4;

    while ((offset - 8) % 16)
	offset --;


    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl << endl;
    // cout << "\t.set\t" << _id->name() << ".size, " << -offset << endl;

    cout << endl;
}

void generateGlobals(const Symbols &globals)
{
    if (globals.size() > 0)
	cout << "\t.data" << endl;

    for (unsigned i = 0; i < globals.size(); i ++) {
	cout << "\t.comm\t"  << globals[i]->name();
	cout << ", " << globals[i]->type().size();
	cout << ", " << globals[i]->type().alignment() << endl;
    }
}