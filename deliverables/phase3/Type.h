// Type.h
# ifndef TYPE_H
# define TYPE_H
# include <string>
# include <vector>
# include <ostream>

typedef std::vector<class Type> Parameters; // create shorthand for parameters


class Type {
    int _specifier;
    unsigned _indirection;
    unsigned _length;
    Parameters *_parameters;
    enum { 
        ARRAY=0, 
        ERROR=1, 
        FUNCTION=2, 
        SCALAR=3 
    } _kind;

public:
    //Constructors
    Type();                                                             //Error Case
    Type(int specifier, unsigned indirection = 0);                      //Declaring a Scalar 
    Type(int specifier, unsigned indirection, unsigned length);         //Declaring an Array
    Type(int specifier, unsigned indirection, Parameters *parameters);  //Declaring a Function

    // Overriding Operators
    bool operator ==(const Type &rhs) const;
    bool operator !=(const Type &rhs) const;

    bool isArray() const;
    bool isScalar() const;
    bool isFunction() const;
    bool isError() const;

    //Getters
    int getKind() const;
    int getSpecifier() const;
    unsigned getIndirection() const;
    unsigned getLength() const;
    Parameters *getParameters() const;
};

std::ostream &operator <<(std::ostream &ostr, const Type &type);

# endif 
