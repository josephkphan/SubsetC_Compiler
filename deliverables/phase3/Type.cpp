# include "Type.h"
# include "tokens.h"
# include <iostream>
# include <string>

using namespace std;

// --------------------------- Constructors --------------------------- //
Type::Type() : _kind(ERROR){} 
Type::Type(int specifier, unsigned indirection) : _specifier(specifier), _indirection(indirection), _kind(SCALAR){}
Type::Type(int specifier, unsigned indirection, unsigned length) : _specifier(specifier), _indirection(indirection), _length(length){_kind = ARRAY;}
Type::Type(int specifier, unsigned indirection, Parameters *parameters) : _specifier(specifier), _indirection(indirection), _parameters(parameters){_kind = FUNCTION;}

// --------------------------- Getters --------------------------- //
int Type::getKind() const {return _kind;}
int Type::getSpecifier() const {return _specifier;}
unsigned Type::getIndirection() const {return _indirection;}
unsigned Type::getLength() const {return _length;}
Parameters *Type::getParameters() const{return _parameters;}

// --------------------------- Checkers --------------------------- //
bool Type::isError() const {return _kind == ERROR;}
bool Type::isScalar() const {return _kind == SCALAR;}
bool Type::isArray() const {return _kind == ARRAY;}
bool Type::isFunction() const {return _kind == FUNCTION;}

// --------------------------- Operators --------------------------- //
bool Type::operator ==(const Type &rhs) const
{
    // Case Error
    if(getKind() != rhs.getKind()){
        return false;
    }
    if (isError()){
        return true;
    }

    // Case Scalar
    if(getSpecifier() != rhs.getSpecifier() || getIndirection() != rhs.getIndirection()){
        return false;
    }
    if (isScalar()){       // Noting that _kind is the same already
        return true;
    }

    // Case Array
    if (isArray()){        // Noting that _kind, _specifier, _indirection are same
        return getLength() == rhs.getLength();
    }
    // Case Function
    // ... You made it this far.. so you have to be a FUNCTION! 
    if (isFunction()){
        return *getParameters() == *rhs.getParameters();
    }
}

bool Type::operator !=(const Type &rhs) const
{
    return !(operator ==(rhs));
}

// --------------------------- Ostream --------------------------- //
ostream &operator <<(ostream &ostr, const Type &type)
{
    string kind_text;
    if(type.isError()){
        kind_text = "Error";
    }else if (type.isScalar()){
        kind_text = "Scalar";
    }else if (type.isArray()){
        kind_text = "Array";
    }else if (type.isFunction()){
        kind_text = "Function";
    }

    ostr << "kind: " << kind_text << " specifier: " << type.getSpecifier() << " indirection: " << type.getIndirection() << endl;
	return ostr;
}



