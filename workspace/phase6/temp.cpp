
void Address::generate()
{
    bool indirect;
    cout << "#ADDRESS" << endl;
    _expr->generate(indirect);
    //_right->generate();

    if(indirect)
    {
        //& → * → p case
        _operand = _expr->_operand; //&*p is just p, so nothing generated
        
    }
    else
    {
        _operand = getTemp();       //get next temp and set it in memory
        cout << "\tleal\t" << _expr << ", %eax" << endl;
        cout << "\tmovl\t%eax, " << this << endl;
    
    }
}

void Dereference::generate(bool &indirect)
{
    indirect = true;
    cout << "#DEREFINDIRECT" << endl;
    _expr->generate();
    _operand = _expr->_operand;
}


void Dereference::generate()
{
    cout << "#DEREF" << endl;
    _expr->generate();
    _operand = getTemp();
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    if(_type.size() == 1)
    {
      cout << "\tmovzbl\t(%eax), %eax" << endl;
      
    }
    else
    {
	cout << "\tmovl\t(%eax), %eax" << endl;
	cout << "\tmovl\t%eax," << this << endl;
    }
    
}

void Negate::generate()
{
    cout << "#NEGATE" << endl;
    _expr->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    cout << "\tnegl\t" << "%eax" << endl;
    //string temp = getTemp();
    cout << "\tmovl\t %eax, " << _operand << endl;
}

void Not::generate()
{
    cout << "#NOT" << endl;
    _expr->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    cout << "\tcmpl\t" << "$0, %eax" << endl;
    cout << "\tsete\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t %eax, " << _operand << endl;
}

void Add::generate()
{
    cout << "#ADD" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\taddl\t" << _right << ", %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl; 
}

void Subtract::generate()
{
    cout << "#SUB" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tsubl\t" << _right << ", %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void Multiply::generate()
{
    cout << "#MULTIPLY" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\timull\t" << _right << ", %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void Remainder::generate()
{
    cout << "#REMAINDER" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tmovl\t" << _right << ", %ecx" << endl;
    cout << "\tcltd" << endl;
    cout << "\tidivl\t" << "%ecx" << endl;
    cout << "\tmovl\t%edx, " << _operand << endl;
}

void Divide::generate()
{	
    cout << "#DIVIDE" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tmovl\t" << _right << ", %ecx" << endl;
    cout << "\tcltd" << endl;
    cout << "\tidivl\t" << "%ecx" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void LessThan::generate()
{
    cout << "#LESS THAN" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetl\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void GreaterThan::generate()
{
    cout << "#GREATER THAN" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetg\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void LessOrEqual::generate()
{
    cout << "#LESS OR EQUAL" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetle\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void GreaterOrEqual::generate()
{
    cout << "#GREATER OR EQUAL" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl; 
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetge\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void Equal::generate()
{
    cout << "#EQUAL" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsete\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void NotEqual::generate()
{
    cout << "#NOT EQUAL" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    
    cout << "\tmovl\t" << _left << ", %eax" << endl;
    cout << "\tcmpl\t" << _right << ", %eax" << endl;
    cout << "\tsetne\t%al" << endl;
    cout << "\tmovzbl\t%al, %eax" << endl;
    cout << "\tmovl\t%eax, " << _operand << endl;
}

void LogicalAnd::generate()
{
    /*
    
    */
    cout << "#LOGICALAND" << endl;
    _left->generate();
    _right->generate();
    _operand = getTemp();
    Label *lbl = new Label();
    if(lbl)
    {
     
      //left
      cout << "\tmovl\t" << _left << ", %eax" << endl;
      cout << "\tcmpl\t$0, %eax" << endl;
      cout << "\tje\t" << *lbl  << endl;
      //right
      cout << "\tmovl\t" << _right << ", %eax" << endl;
      cout << "\tcmpl\t$0, %eax" << endl;
      
      //LABEL
      cout << *lbl << ":" << endl;
      cout << "\tsetne\t%al" << endl;
      cout << "\tmovzbl\t%al, %eax" << endl;
      cout << "\tmovl\t%eax, " << _operand << endl;
    }
}

void LogicalOr::generate()
{
    /*
    
    */
    //_left->generate();
    //_right->generate();
    cout << "#LOGICALOR" << endl;
    _operand = getTemp();
    Label *lbl = new Label();
    if(lbl)
    { 
      
      //left
      _left->generate();
      cout << "\tmovl\t" << _left << ", %eax" << endl;
      cout << "\tcmpl\t$0, %eax" << endl;
      cout << "\tjne\t" << *lbl  << endl;
      _right->generate();
      //right
      cout << "\tmovl\t" << _right << ", %eax" << endl;
      cout << "\tcmpl\t$0, %eax" << endl;
      
      //LABEL
      cout << *lbl << ":" << endl;
      cout << "\tsetne\t%al" << endl;
      cout << "\tmovzbl\t%al, %eax" << endl;
      cout << "\tmovl\t%eax, " << _operand << endl;
    }
}

void Return::generate()
{
    /*
    _operand = *retlbl;
    */
    //need global Label *retlbl;
    cout << "#RETURN" << endl;
    _expr->generate();
   
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    cout << "\tjmp\t" << *retlbl << endl;
}

void String::generate()
{
    /*
    print this: .l3:    .asciz "%d\n"
    in use:     leal    .l3, %eax
    */

    cout << "#STRING" << endl;
    stringstream ss;
    Label *lbl = new Label();
    ss << *lbl;
    _operand = ss.str();
    ss << ":\t.asciz " << _value << endl;
    strs.push_back(ss.str());
    
    
}


void Promote::generate()
{
    /*
    if size == 1, extend value to long and store into a temp
    */ 
    //sign extend value of subexpression
    cout << "#PROMOTE" << endl;
    _expr->generate();
    
    if(_expr->type().size() == 1)
    {
        //extend value to long, store into temp
        _operand = getTemp();
        cout << "\tmovsbl\t" << _expr << ", %eax" << endl;
        cout << "\tmovl\t" << "%eax, " << _operand << endl;
    }
}

void While::generate()
{
    /*
    in notes
    */
    Label *loop = new Label();
    Label *exitloop = new Label();

    cout << "#WHILE" << endl;
    cout << *loop << ":" << endl;
    _expr->generate();
    
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    cout << "\tcmpl\t$0, %eax" << endl;
    cout << "\tje\t" << *exitloop << endl;
    _stmt->generate();
    cout << "\tjmp\t" << *loop << endl;
    cout << *exitloop << ":" << endl;
    
}

void For::generate()
{
    /*
    
    */
    Label *loop = new Label();
    Label *exitloop = new Label();

    cout << "#FOR" << endl;
    _init->generate();
    cout << *loop << ":" << endl;
    
    _expr->generate();
    
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    cout << "\tcmpl\t$0, %eax" << endl;
    cout << "\tje\t" << *exitloop << endl;
    _stmt->generate();
    _incr->generate();
    cout << "\tjmp\t" << *loop << endl;
    cout << *exitloop << ":" << endl;



    
}

void If::generate()
{
    /*
    in notes
    */
    Label *lbl = new Label();
    Label *lblelse = new Label();
    cout << "#IF" << endl;
    _expr->generate();
    cout << "\tmovl\t" << _expr << ", %eax" << endl;
    cout << "\tcmpl\t$0, %eax" << endl;
    cout << "\tje\t" << *lblelse << endl;
    _thenStmt->generate();
    cout << "\tjmp\t" << *lbl << endl;
    
    cout << *lblelse << ":" << endl;
    if(_elseStmt)
    {
        _elseStmt->generate();
    }
    cout << *lbl << ":" << endl;
}