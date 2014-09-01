#ifndef _GLOBAL_H_
#define _GLOBAL_H_

char *token_str[] = {
	"#",
#define TSYM(tk, s)	s,
#include "tsym.h"
#undef TSYM
#define NONTSYM(tk, s) s,
#include "nontsym.h"
#undef NONTSYM
};

struct input input;
int value;
int no = 1;
int tb_index[SYM_SIZE + 1] = {0};
int *ff_index[NONTERMINAL_SYMBOL_SIZE][2];
int first[] = {
#include "first.h"
};

int follow[] = {
#include "follow.h"
};

struct set set[SET_SIZE] = {
#include "set.h"
};

struct table table[TABLE_SIZE] = {
#include "table.h"
};

struct stack {
	int code;
	int dp; //选择产生式在set中的相对索引
} stack[STACK_SIZE] = {0};

int top = -1;
int next_tk;
#endif
