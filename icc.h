#ifndef _CC_H_
#define _CC_H_

#include "./include/err.h"
#define END_OF_FILE -1
#define TERMINAL_SYMBOL_SIZE	75
#define NONTERMINAL_SYMBOL_SIZE	68
#define SYM_SIZE 143
#define FIRST_SIZE 687 
#define FOLLOW_SIZE 1053 
#define SET_SIZE 211
#define TABLE_SIZE 492
#define STACK_SIZE 128 

#define IS_OPERATOR(tk) (((tk) >= 26 && (tk) <= 50) ||\
		(tk) == NTS_ASSIGNMENT_OPTOR || (tk) == NTS_UNARY_OPTOR)

#define ishex(ch)	((ch) >= '0' && (ch) <= '9' ||\
		(ch) >= 'a' && (ch) <= 'f' ||\
		(ch) >= 'A' && (ch) <= 'F')

#define isoctal(ch) ((ch) >= '0' && (ch) <= '7')

enum token_tag{
	ZERO = 0,
#define TSYM(tk, s)	tk,
#include "./include/tsym.h"
#undef TSYM
#define NONTSYM(tk, s) tk,
#include "./include/nontsym.h"
#undef NONTSYM
};

struct input
{
	char ch;
    char *filename;
    FILE *fp;
};

struct table{
	int code;
	int s_index;
	int r_index;
};

struct set {
	int code;
	int right[10];
	int (*look_back)(int index);
};

int get_next_token(void);
void read_source(char *filename);
void close_source(void);
#endif
