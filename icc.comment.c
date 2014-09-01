#include <stdio.h>
#include <string.h>
#include <malloc.h> 
#include "icc.h"
#include "global.h"

//查找fisrt或follow时传递参数宏
#define FOLLOW 0
#define FIRST 1

//输出从栈底到栈顶
void output_stack(void)
{
	int i;
	for(i = 0; i <= top; ++i)
		printf("%s ", token_str[stack[i].code]);
	printf("\n");
}

//对struct table的索引tb_index的初始化
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

//初始化follow基first基索引表ff_index
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

//查找follow或first基，以参数ff区分。长度大于5的折半查，<=5蛮力查
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

//*自身向上归约钩子函数
int mul_star(int i)
{
	struct set *s;
	int nr;
	int code;
	int flag = 0;
	int j;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 
	
	//向栈后看特定符号，设置对应标志
	for(j = top - 1; j > top - 5; --j) {
		if(stack[j].code == NTS_DECLON_SPF || 
				stack[j].code == NTS_SPF_QUAL_L ||
				stack[j].code == TS_MUL) {
			flag = 1;
			break;
		}
	}

	//根据设置的标识选择产生式归约
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

//type-specifier自身向上归约钩子函数
int type_specifier(int i)
{
	struct set *s;
	int nr;
	int code;
	int tmp;
	int spf_f, declon_f;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	//向栈后看特定符号，设置对应标志
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

	//根据设置的标识选择产生式归约
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

//id自身向上归约钩子函数
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

	//向栈后看特定符号，设置对应标志
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

	//根据设置的标识选择产生式归约
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

//declarator自身向上归约钩子函数
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

	//向栈后看特定符号，设置对应标志
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

	//根据设置的标识选择产生式归约
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

//declaration自身向上归约钩子函数
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

//conditional_exp自身向上归约钩子函数
int conditional_exp(int i)
{
	struct set *s;
	int tmp;
	int nr;
	int code;
	int flag;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	//向栈后看特定符号，设置对应标志
	tmp = 1;
	flag = 0;
	while(tmp < 5 && (top - tmp) >= 0) {
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

	//根据设置的标识选择产生式归约
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

//assignment_exp自身向上归约钩子函数
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

	//向栈后看特定符号，设置对应标志
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

	//根据设置的标识选择产生式归约
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

/*需要向后看才能决定自身向上归约方向；一共7个，一个不多，一个也不少
 * 就这几个！，其余都不需要向后看就能在主归约函数内处理完成!
 * 它们即所谓的钩子函数了，每个钩子函数处理过程都一致！过程如下：
 * 一，根据向后看到的具体内容，设置对应的归约标识
 * 二，搜索剩余产生式（传递的i用来识别剩余产生式的起始）
 * 三，根据第一步的归约标识执行相应归约*/
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

/*不需要归约的终结符直接移进,不用做任何归约处理*/
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

/*向前看决定是归约还是移进, 这里的归约不包含自身向上归约，
 *主要是解决一个产生式是另外一个产生式的前缀时是归约还是移进
 *以及一个产生式是另一个产生式后缀, 此时是继续自身向上归约
 *（往下走,不是在这里自身向上归约）还是结合已经匹配的产生式进行
 *归约，还是移进*/
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
						return 1; /*struct union
					enum类型声明，则移进下一个tk,否则是变量定义，则归约*/
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

	//归约
	if(s->right[tb->r_index + 1] == 0) {
		top -= tb->r_index;
		stack[top].code = s->code;
		return 0;
	}

	if(!gn)
		return 1; 

	return 2;

declor: //下面解决declarator归约冲突
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

/*检测first基*/
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

/*主归约函数
 *0-归约成功继续下一次归约，1-无归约移入新tk，-1-错误
 */
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

	if(top > 0) { //归约栈不空
		for(i = 0; i < nr; ++i) { //匹配产生式
			tb = table + tb_index[code] + i;
			s = set + tb->s_index;
			if(tb->r_index == 0) //如果产生式不需要向栈后匹配，则跳出循环
				break;
			for(tmp = tb->r_index - 1, j = top - 1; 
					j >= 0 && tmp >= 0; --tmp, --j) {//向栈后匹配产生式
				if(s->right[tmp] != stack[j].code)
					break;  //不匹配则循环结束
			}

			if(tmp == -1) { //向栈后匹配成功
				ret = look_front(i); //向前看，决定归约还是移进
				if(ret != 2) { //返回值ret不等2，即返回0或者1时
					if(ret == 1) //返回1，则需要移进，则检测first基
						ret = check_in(i);
					return ret; //ret== 0 || ret == 1 ，退出函数main_reduction
				}
			}
			//tmp == -1 && ret = = 2 或者tmp != -1继续匹配下一个产生式
		}
	}
	else { //归约栈空
		for(i = 0; i < nr; ++i) { //跳过产生式右部索引r_index不为0的产生式
			tb = table + tb_index[code] + i;
			s = set + tb->s_index;
			if(tb->r_index == 0) 
				break;
		}
	}

	//跳过需要移进下一个tk的产生式
	for(; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		if(s->right[1] == 0)
			break;
	}

	//如果不存在自身向上归约的产生式，则移进下一个tk
	if(i == nr)
		return 1;

	/*如果仅有一条自身向上归约的产生式，且next_tk是产生式左部的follow，
	  则归约*/
	if(i + 1 == nr) {
		if(is_ff(s->code, next_tk, FOLLOW)) {
			stack[top].code = s->code;
			return 0;
		}
		else//否则移进下一个tk
			return 1;
	}

	//自身向上归约
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
	while(!finished) { //归约未结束
		//栈溢出，出错
		if(top >= STACK_SIZE - 1)
			return -1;

		//移进下一个tk
		code = stack[++top].code = next_tk;
		next_tk = get_next_token();
		if(next_tk == -1)
			next_tk = 0;

		printf("\ncurr: %s   next: %s\n", 
				token_str[code], token_str[next_tk]);

		while(over == 0) {
			code = stack[top].code;
			switch(code) {
				case NTS_TRANS_UNIT: //归约树根出现，设置标志
					over = 2;
					break;
				case NTS_FUN_DEF_L: //这里是语法所致
					if(next_tk != 0) {
						over = 1;
						break;
					}
				default:
					over = main_reduction(code); //归约处理
			}

			if(!over) //归约成功，则输出栈信息
				output_stack();
		}

		switch(over) {
			case -1: //出错，输出出错信息
				printf("error: line no %d , %s\n", no, token_str[next_tk]);
				return -1;
			case 2: //检测是否接受程序
				finished = 1;
				if(next_tk == 0) {
					if(top == 0)
						return 0;
				}
				break;
			default: //重设标志，准备下一次循环
				over = 0;
		}
	}

	return -1; //出错
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
