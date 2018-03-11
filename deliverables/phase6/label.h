 
# ifndef LABEL_H
# define LABEL_H


struct Label {
    static unsigned _counter;
    unsigned _number;

public:
    Label();
    unsigned number() const;
};

# endif