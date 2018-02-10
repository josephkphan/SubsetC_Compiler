all: 
	lex -t program.l > lex.cpp
	g++ -o scc lex.cpp
	./scc < test.c > test.txt
	diff test.txt test.out
clean:
	rm scc lex.cpp
