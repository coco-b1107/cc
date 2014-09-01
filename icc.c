#include <stdio.h>
#include <string.h>
#include <malloc.h> 
#include "icc.h"
#include "global.h"

#define FOLLOW 0
#define FIRST 1

void output_stack(void)
{
	int i;
	for(i = 0; i <= top; ++i)
		printf("%s ", token_str[stack[i].code]);
	printf("\n");
}

static void init_tb_index(void)
{
	int i, j = 2;
	int temp;
	temp = table[0].code;
	tb_index[1] = 0;

	for(i = 1; i < TABLE_SIZE; ++i) {
		if(table[i].code != temp) {
			temp = table[i].code; 
			tb_index[j++] = i;
		}
	}
}

void init_ff_index(void)
{
	int i, j;
	int code;

	i = j = 0;
	for(code = 0; code < NONTERMINAL_SYMBOL_SIZE; ++code, ++i, ++j) {
		ff_index[code][0] = follow + i;
		ff_index[code][1] = first + j;
		i += follow[i];
		j += first[j];
	}
}

static inline int is_ff(int code, int ts, int ff)
{
	int *f;
	int start, end, mid;
	
	f = ff_index[code - TERMINAL_SYMBOL_SIZE - 1][ff];
	if(f[0] > 5) {
		start = 0; end = f[0] - 1;
		++f;
		while(end >= start) {
			mid = (start + end) >> 1;
			if(f[mid] == ts)
				return 1;
			if(f[mid] > ts) 
				end = mid - 1;
			else
				start = mid + 1;
		}
		if(end < start)
			return 0;
	}
	else {
		for(end = f[0], ++f; end; --end, ++f){
			if(*f == ts)
				return 1;
		}
		return 0;
	}
}

int mul_star(int i)
{
	struct set *s;
	int nr;
	int code;
	int flag = 0;
	int j;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	for(j = top - 1; j > top - 5; --j) {
		if(stack[j].code == NTS_DECLON_SPF || 
				stack[j].code == NTS_SPF_QUAL_L ||
				stack[j].code == TS_MUL) {
			flag = 1;
			break;
		}
	}
	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		if(is_ff(s->code, next_tk, FOLLOW)) {
			switch(s->code) {
				case NTS_POINTER:
					if(flag) {
						stack[top].code = s->code;
						return 0;
					}
					break;

				case NTS_UNARY_OPTOR:
					if(!flag) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				default:
					return -1;
			}
		}
	}
	return 1;
}

int type_specifier(int i)
{
	struct set *s;
	int nr;
	int code;
	int tmp;
	int spf_f, declon_f;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	spf_f = declon_f = 0;
	tmp = 1; 
	while(tmp < 5 && (top - tmp) >= 0) {
		switch(stack[top - tmp].code) {
			case TS_STRUCT:
			case TS_UNION:
			case TS_SIZEOF:
			case NTS_STRUCT_DECLON_L:
			case NTS_ASSIGNMENT_OPTOR:
				spf_f = 1;
				tmp = 5;
				break;
			case TS_COMMA:
			case NTS_DECLON_L:
			case NTS_FUN_DEF_L:
			case NTS_STORE_CLASS_SPF:
			case NTS_DECLOR:
			case NTS_DECLON_SPF:
				declon_f = 1;
				tmp = 5;
				break;
			case TS_ID:
			case TS_LBRACE:
			case TS_LPAREN:
			case NTS_TP_SPF:
				break;
		}
		++tmp;
	}

	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		if(is_ff(s->code, next_tk, FOLLOW)) { 
			if(top) {
				switch(s->code) {
					case NTS_SPF_QUAL_L:
						if(spf_f) {
							stack[top].code = s->code;
							return 0;
						}
						break;
					case NTS_DECLON_SPF:
						if(declon_f) {
							stack[top].code = s->code;
							return 0;
						}
						break;
					default:
						return -1;
				}
			} else if(s->code == NTS_DECLON_SPF) {
				stack[top].code = s->code;
				return 0;
			}
		}
	}
	return 1;
}

int id(int i)
{
	struct set *s;
	int nr;
	int code;
	int tmp;
	int direct_f, enumon_f, primary_f;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 
	tmp = 1;
	direct_f = enumon_f = primary_f = 0;

	while(tmp < 3 && (top - tmp) >= 0) {
		switch(stack[top - tmp].code) {
			case NTS_POINTER:
			case NTS_DECLON_SPF:
			case NTS_DECLON_L:
			case NTS_SPF_QUAL_L:
			case NTS_INIT_DECLOR_L:
			case NTS_STRUCT_DECLOR_L:
				direct_f = 1;
				tmp = 3;
				break;
			case TS_ID:
			case NTS_ENUMOR_L:
				enumon_f = 1;
				tmp = 3;
				break;
			case TS_LBRACE:
			case TS_COMMA:
				break;
			default:
				primary_f = 1;
				tmp = 3;
				break;
		}
		++tmp;
	}

	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		if(is_ff(s->code, next_tk, FOLLOW)) {
			if(top) {
				switch(s->code) {
					case NTS_DIRECT_DECLOR:
						if(direct_f) {
							stack[top].code = s->code;
							return 0;
						}
						break;
					case NTS_ENUMON_CONST:
						if(enumon_f) {
							stack[top].code = s->code;
							return 0;
						}
						break;
					case NTS_PRIMARY_EXP:
						if(primary_f) {
							stack[top].code = s->code;
							return 0;
						}
						break;
					case NTS_TYPEDEF_NAME:
						break;
					default:
						return -1;
				}

			} else if(s->code == NTS_PRIMARY_EXP) {
				stack[top].code = s->code;
				return 0;
			}
		}
	}
	return 1;
}

int declarator(int i)
{
	struct set *s;
	int nr;
	int code;
	int tmp;
	int flag;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 
	tmp = 1;
	flag = 0;
	while(tmp < 3 && (top - tmp) >= 0) {
		switch(stack[top - tmp].code) {
			case NTS_SPF_QUAL_L:
				flag = 1;
				tmp = 3;
				break;
			case NTS_DECLON_SPF:
			case NTS_INIT_DECLOR_L:
				flag = 0;
				tmp = 3;
				break;
			case TS_COMMA:
				break;
		}
		++tmp;
	}

	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		if(is_ff(s->code, next_tk, FOLLOW)) {
			switch(s->code) {
				case NTS_STRUCT_DECLOR_L:
					if(flag) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				case NTS_INIT_DECLOR:
					if(!flag) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				default:
					return -1;

			}
		}
	}
	return 1;
}

int declaration(int i)
{
	struct set *s;
	int nr;
	int code;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		
		if(is_ff(s->code, next_tk, FOLLOW)) { 
			if(top == 0 && s->code == NTS_DECLON_L) {
				stack[top].code = s->code;
				return 0;
			}
			if(top > 0) {
				switch(stack[top - 1].code) {
					case NTS_BLOCK_ITEM_L:
					case TS_LBRACE:
						if(s->code == NTS_BLOCK_ITEM) {
							stack[top].code = s->code;
							return 0;
						}
						break;
					default:
						if(s->code == NTS_DECLON_L) {
							stack[top].code = s->code;
							return 0;
						}
				}
			}
		}
	}
	return 1;
}

int conditional_exp(int i)
{
	struct set *s;
	int tmp;
	int nr;
	int code;
	int flag;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	tmp = 1;
	flag = 0;
	while(tmp < 5) {
		switch(stack[top - tmp].code) {
			case TS_CASE:
			case NTS_ENUMON_CONST:
			case NTS_DESIGOR_L:
				flag = 1;
				tmp = 5;
				break;
			case NTS_DECLOR:
			case NTS_DESIGON:
			case NTS_INITER_L:
				flag = 0;
				tmp = 5;
				break;
			case TS_ASSIGN:
			case TS_COMMA:
				break;
			case TS_LBRAKET:
				if(stack[top - tmp - 1].code == TS_LBRACE ||
				stack[top - tmp - 1].code == TS_COMMA) {
					flag = 1;
					tmp = 5;
				}
			case TS_LBRACE:
				break;
		}
		++tmp;
	}
	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		if(is_ff(s->code, next_tk, FOLLOW)) { 
			switch(s->code) {
				case NTS_CONST_EXP:
					if(flag) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				case NTS_ASSIGNMENT_EXP:
					if(!flag) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				default:
					return -1;
			}
		}
	}
	return 1;
}

int assignment_exp(int i)
{
	struct set *s;
	int tmp;
	int nr;
	int code;
	int exp_f, initer_f, argument_f;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 
	exp_f = initer_f = argument_f = 0;

	tmp = 1;
	while(tmp < 4) {
		switch(stack[top - tmp].code) {
			case TS_SEMICOLON:
			case TS_RETURN:
			case TS_QUESTION:
			case TS_RPAREN:
			case TS_DO:
			case TS_WHILE:
			case TS_FOR:
			case TS_IF:
			case TS_ELSE:
			case TS_SWITCH:
			case TS_AND:
			case TS_OR:
			case TS_COLON:
			case NTS_BLOCK_ITEM_L:
			case NTS_POSTFIX_EXP:
			case NTS_ASSIGNMENT_OPTOR:
				exp_f = 1;
				tmp = 4;
				break;
			case NTS_DESIGON:
			case NTS_DECLOR:
			case NTS_INITER_L:
			case NTS_INIT_DECLOR_L:
				initer_f = 1;
				tmp = 4;
				break;
			case NTS_ARGUMENT_EXP_L:
				argument_f = 1;
				tmp = 4;
				break;
			case TS_LPAREN:
				if(stack[top - tmp - 1].code == NTS_POSTFIX_EXP) {
						argument_f = 1;
						tmp = 4;
						break;
				}
			case TS_COMMA:
			case TS_LBRAKET:
			case TS_LBRACE:
				switch(stack[top - tmp - 1].code) {
					case TS_ASSIGN:
						initer_f = 1;
						tmp = 4;
						break;
					case NTS_DECLOR:
						exp_f = 1;
						tmp = 4;
						break;
				}
				break;
			default:
				if(IS_OPERATOR(stack[top - tmp].code)) {
					exp_f = 1;
					tmp = 4;
				}
		}
		++tmp;
	}

	for(; i < nr; ++i) {
		s = set + table[tb_index[code] + i].s_index;
		if(is_ff(s->code, next_tk, FOLLOW)) { 
			switch(s->code) {
				case NTS_INITER:
					if(initer_f) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				case NTS_EXP:
					if(exp_f) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				case NTS_ARGUMENT_EXP_L:
					if(argument_f) {
						stack[top].code = s->code;
						return 0;
					}
					break;
				default:
					return -1;
			}
		}
	}
	return 1;
}

static int look_back(int i)
{
	int ret;

	switch(stack[top].code) {
		case TS_MUL:
			ret = mul_star(i);
			break;
		case TS_ID:
			ret = id(i);
			break;
		case NTS_ASSIGNMENT_EXP:
			ret = assignment_exp(i);
			break;
		case NTS_COND_EXP:
			ret = conditional_exp(i);
			break;
		case NTS_DECLON:
			ret = declaration(i);
			break;
		case NTS_DECLOR:
			ret = declarator(i);
			break;
		case NTS_TP_SPF:
			ret = type_specifier(i);
			break;
		default:
			ret = -1;
	}

	return ret;
}

static inline int is_direct_in(int code)
{
	switch(code) {
		case TS_STRUCT:
		case TS_UNION:
		case TS_ENUM:
		case TS_GOTO:
		case TS_CONTINUE:
		case TS_BREAK:
		case TS_RETURN:
		case TS_CASE:
		case TS_DEFAULT:
		case TS_DO:
		case TS_WHILE:
		case TS_FOR:
		case TS_IF:
		case TS_ELSE:
		case TS_SWITCH:
		case TS_DOT:
		case TS_POINTER:
		case TS_SIZEOF:
		case TS_DIV:
		case TS_MOD:
		case TS_LSHIFT:
		case TS_RSHIFT:
		case TS_LESS:
		case TS_GREATER:
		case TS_GREATER_EQ:
		case TS_LESS_EQ:
		case TS_EQ:
		case TS_NOT_EQ:
		case TS_BITXOR:
		case TS_BITOR:
		case TS_AND:
		case TS_OR:
		case TS_QUESTION:
		case TS_COLON:
		case TS_LBRACE:
		case TS_LPAREN:
		case TS_LBRAKET:
			return 1;
		default:
			return 0;
	}
}

static int look_front(int i)
{
	struct set *s;
	struct table *tb;
	int code;
	int gn;

	code = stack[top].code;
	tb = table + tb_index[code] + i;
	s = set + tb->s_index;
	gn = 0;
	switch(code) {
		case TS_ID:
			switch(stack[top - 1].code) {
				case TS_STRUCT:
				case TS_UNION:
				case TS_ENUM:
					if(next_tk == TS_LBRACE)
						return 1;
					top -= tb->r_index;
					stack[top].code = s->code;
					return 0;
			}
			break;
		case NTS_DIRECT_DECLOR:
			if(next_tk == TS_LBRAKET || next_tk == TS_LPAREN)
				return 1;
			break;
		case NTS_STM:
			if(next_tk == TS_ELSE) //下一个是else移进
				return 1;
			gn = 1; //否则，继续匹配下一个产生式
			break;
		case NTS_ASSIGNMENT_EXP:
			if(stack[top - 1].code == TS_LBRAKET && 
					stack[top - 2].code == NTS_POSTFIX_EXP)
				//[assign-exp继续归约，还是移进解决归约direct-abstruct-dec冲突
				gn = 1;
			break;
		case NTS_DECLOR:
			goto declor;
		default:
			break;
	}

	if(s->right[tb->r_index + 1] == 0) {
		top -= tb->r_index;
		stack[top].code = s->code;
		return 0;
	}

	if(!gn)
		return 1; 

	return 2;

declor:
	switch(stack[top - 1].code) {
		case NTS_DECLON_SPF:
			if(next_tk == TS_LBRACE)
				return 1;
			if(!s->right[tb->r_index + 1] && 
					(top - 3) > 0) {
				if(stack[top - 3].code == NTS_DIRECT_DECLOR ||
						stack[top - 3].code == NTS_PRMT_L) {
					top -= tb->r_index;
					stack[top].code = s->code;
					return 0;
				}
			}
			break;
		case TS_COMMA:
			top -= tb->r_index;
			stack[top].code = s->code;
			return 0;
		case TS_LPAREN:
			return 1;
	}

	return 2;
}

static int check_in(int i)
{
	struct set *s;
	struct table *tb;
	int code;

	code = stack[top].code;

	tb = table + tb_index[code] + i;
	s = set + tb->s_index;
	code = s->right[tb->r_index + 1];

	if(code <= TERMINAL_SYMBOL_SIZE)
		return 1;

	if(is_ff(code, next_tk, FIRST))
		return 1;
	else
		return -1;
}

//0-归约成功继续下一次归约，1-无归约移入新tk，-1-错误
int main_reduction(int code)
{
	struct set *s;
	struct table *tb;
	int i, j;
	int tmp;
	int nr;
	int ret;

	if(is_direct_in(code))
		return 1;

	nr = tb_index[code + 1] - tb_index[code]; 

	if(top > 0) {
		for(i = 0; i < nr; ++i) { 
			tb = table + tb_index[code] + i;
			s = set + tb->s_index;
			if(tb->r_index == 0) 
				break;
			for(tmp = tb->r_index - 1, j = top - 1; 
					j >= 0 && tmp >= 0; --tmp, --j) {
				if(s->right[tmp] != stack[j].code)
					break; 
			}

			if(tmp == -1) { 
				ret = look_front(i);
				if(ret != 2) {
					if(ret == 1)
						ret = check_in(i);
					return ret;
				}
			}
		}
	}
	else {
		for(i = 0; i < nr; ++i) { 
			tb = table + tb_index[code] + i;
			s = set + tb->s_index;
			if(tb->r_index == 0) 
				break;
		}
	}

	for(; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		if(s->right[1] == 0)
			break;
	}

	if(i == nr)
		return 1;

	if(i + 1 == nr) {
		if(is_ff(s->code, next_tk, FOLLOW)) {
			stack[top].code = s->code;
			return 0;
		}
		else
			return 1;
	}
	return look_back(i);
}

int LR(void)
{
	int finished = 0;
	int over = 0;
	int nr;
	int i, j;
	int *set;
	int code;
	char *str = "id";

	next_tk = get_next_token();
	while(!finished) {
		if(top >= STACK_SIZE - 1)
			return -1;
		code = stack[++top].code = next_tk;
		next_tk = get_next_token();
		if(next_tk == -1)
			next_tk = 0;
		printf("\ncurr: %s   next: %s\n", 
				token_str[code], token_str[next_tk]);

		while(over == 0) {
			code = stack[top].code;
			switch(code) {
				case NTS_TRANS_UNIT:
					over = 2;
					break;
				case NTS_FUN_DEF_L:
					if(next_tk != 0) {
						over = 1;
						break;
					}
				default:
					over = main_reduction(code);
			}
			if(!over)
				output_stack();

		}

		switch(over) {
			case -1:
				printf("error: line no %d , %s\n", no, token_str[next_tk]);
				return -1;
			case 2:
				finished = 1;
				if(next_tk == 0) {
					if(top == 0)
						return 0;
				}
				break;
			default:
				over = 0;
		}
	}
	return -1;
}

int main(int argc, char *argv[]) 
{
	init_tb_index();
	init_ff_index();
	read_source(argv[1]);
	printf("ret: %d\n", LR());
	close_source();
	return 0;
}
