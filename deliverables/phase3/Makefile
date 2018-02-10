CXX		= g++
CXXFLAGS	= -g -Wall
EXTRAS		= lexer.cpp
OBJS		= lexer.o parser.o Type.o Symbol.o Scope.o checker.o
PROG		= scc


all:		$(PROG)

$(PROG):	$(EXTRAS) $(OBJS)
		$(CXX) -o $(PROG) $(OBJS)

clean:;		$(RM) $(PROG) core *.o

clobber:;	$(RM) $(EXTRAS) $(PROG) core *.o

lexer.cpp:	lexer.l
		$(LEX) $(LFLAGS) -t lexer.l > lexer.cpp

lextest:	lextest.cpp lexer.o
		$(CXX) -o lextest lextest.cpp lexer.o
