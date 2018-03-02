CXX		= g++
CXXFLAGS	= -g -Wall -Wno-c++0x-compat
EXTRAS		= lexer.cpp
OBJS		= Scope.o Symbol.o Tree.o Type.o checker.o lexer.o parser.o generator.o
PROG		= scc


all:		$(PROG)

$(PROG):	$(EXTRAS) $(OBJS)
		$(CXX) -o $(PROG) $(OBJS)

clean:;		$(RM) $(PROG) core *.o

clobber:;	$(RM) $(EXTRAS) $(PROG) core *.o

lexer.cpp:	lexer.l
		$(LEX) $(LFLAGS) -t lexer.l > lexer.cpp
