INCLUDE=./include
icc: icc.o lex.o
	cc -o icc icc.o lex.o
lex.o: ./lex/lex.c icc.h ${INCLUDE}/tsym.h ${INCLUDE}/nontsym.h
	cc -c -g ./lex/lex.c
icc.o: icc.c icc.h ${INCLUDE}/tsym.h ${INCLUDE}/nontsym.h \
		${INCLUDE}/global.h ${INCLUDE}/set.h ${INCLUDE}/table.h \
		${INCLUDE}/first.h ${INCLUDE}/follow.h
	cc -c -g icc.c
clean:
	-rm icc *o
