# include <iostream>
# include "generator.h"
# include "Symbol.h"

using namespace std;

static int argument_counter;

// ---------------- For Debugging & Printing ---------------- //

ostream &operator <<(ostream &ostr, Expression *expr){
    return ostr << expr->_operand;
}

void generator_debug_log(string message){
    // cout << "--" << message << endl;         //TODO: COMMENT OUT ON SUBMISSION
}

// --------------------- Actual Logic ---------------------- //

//Step 1: Function Generate 
void Function::allocate(int &offset)  {
    generator_debug_log("Function:allocate - start");
    Symbols syms = _body->declarations()->symbols();
    int paramCount, local_variable_counter,parameter_offset,local_variable_offset, math_temp;
    paramCount = _id->type().parameters()->_types.size();
    parameter_offset = 8;
    local_variable_counter = syms.size();
    local_variable_offset = 0;


    for (int i = 0; i < paramCount; i++) {
        generator_debug_log("Function:allocate - param offset: " + std::to_string(parameter_offset));
        syms[i]->_offset = parameter_offset;
        parameter_offset += 4;
    }

    for (int i = paramCount; i < local_variable_counter; i++) {
        math_temp = 1;

        if (syms[i]->type().isArray()) {
            math_temp = syms[i]->type().length();
        }

        local_variable_offset -= math_temp * 4;
        syms[i]->_offset = local_variable_offset;
    }
    offset = local_variable_offset;
    generator_debug_log("Function:allocate - End Offset: " + std::to_string(offset));
    generator_debug_log("Function:allocate - end");
}

void Function::generate(){
    generator_debug_log("Function:generate - start");
    int offset = 0;

    allocate(offset);

    cout << "\t.globl\t" << _id->name() << endl;
    cout << _id->name() << ":" << endl;
    cout << "\tpushl\t%ebp" << endl;
    cout << "\tmovl\t%esp, %ebp" << endl;
    cout << "\tsubl\t$" << -offset << ", %esp" << endl;

    argument_counter = 0;
    _body->generate();

    offset -= argument_counter * 4;

    while ((offset - 8) % 16){
        offset --;
    }

    cout << "\tmovl\t%ebp, %esp" << endl;
    cout << "\tpopl\t%ebp" << endl;
    cout << "\tret" << endl << endl << endl;

    generator_debug_log("Function:generate - end");
}

// Step 2: Print Globals 
void printGlobals(const Symbols &globals){
    generator_debug_log("printGlobals - start");
    if (globals.size() > 0){
        cout << "\t.data" << endl;
    }

    for (int i = 0; i < globals.size(); i ++) {
        cout << "\t.comm\t"  << globals[i]->name() << ", " << globals[i]->type().size() << ", " << globals[i]->type().alignment() << endl;
    }
    generator_debug_log("printGlobals - end");
}

// Step 3: add offset to Symbols  -- DONE
// Step 4: Modify Type.size -- DONE
// Step 5: add _operand to expr -- DONE

// Step 6: Block Generate
void Block::generate(){
    generator_debug_log("Block:generate - start");
    for (int i = 0; i < _stmts.size(); i ++){
        _stmts[i]->generate();
    }
    generator_debug_log("Block:generate - end");
}

// Step 7: Number Generate 
void Number::generate(){
    generator_debug_log("Number:generate - start");
    _operand = "$" + _value;
    generator_debug_log("Number:generate - end");
}

//Step 8: Identifier Generate
void Identifier::generate(){
    generator_debug_log("Identifier:generate - start");
    if (_symbol->_offset != 0){
        _operand = std::to_string(_symbol->_offset) + "(%ebp)";
    }else{
        _operand = _symbol->name();
    }
    generator_debug_log("Identifier:generate - end");
}

// Step 9: Assign Generate
void Assignment::generate(){
    generator_debug_log("Assignment:generate - start");
    _left->generate();
    _right->generate();

    cout << "\tmovl\t" << _right << ", %eax" << endl;
    cout << "\tmovl\t%eax, " << _left << endl;
    generator_debug_log("Assignment:generate - end");
}

//Step 10: Call Generate
void Call::generate(){
    generator_debug_log("Call:generate - start");
    int numBytes = 0;
    for (int i = _args.size() - 1; i >= 0; i --) {
        _args[i]->generate();
        if (_args[i]->type().isInteger()){
            cout << "\tpushl\t" << _args[i]->_operand << endl;
        }else {
            cout << "\tpushl\t-" << numBytes + _args[i]->type().size() << "(%ebp)" << endl;
        }
        numBytes += _args[i]->type().size();
    }
    cout << "\tcall\t"  << _id->name() << endl;
    if (numBytes > 0){
        cout << "\taddl\t$" << numBytes << ", %esp" << endl;
    }else {
        generator_debug_log("Call:generate - snumBytes <= 0");
    }
    generator_debug_log("Call:generate - end");
}

