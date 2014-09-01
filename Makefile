icc: icc.o lex.o
	cc -o icc icc.o lex.o
lex.o: lex.c icc.h tsym.h nontsym.h
	cc -c -g lex.c
icc.o: icc.c icc.h tsym.h nontsym.h global.h set.h table.h first.h follow.h
	cc -c -g icc.c

clean:
	-rm icc *o
