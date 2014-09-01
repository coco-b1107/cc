/*
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

void init_ff(void)
{
	int i, j;
	struct set *s;
	int code = 76;

	s = set;
	i = j = 0;
	for(code = 76; code <= SYM_SIZE; ++code, ++i, ++j) {
		for(; s->code == code; ++s) {
			s->first = first + i;
			s->follow = follow + j;
		}
		i += first[i];
		j += follow[j];
	}
}

static  int is_follow(struct set *s, int ts)
{
	int *f;
	int start, end, mid;
	
	f = s->follow;
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
	struct table *tb;
	int nr;
	int code;
	int flag = 0;
	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	for(i = top - 1; i >= 0; --i) {
		if(stack[i].code == NTS_DECLON_SPF || 
				stack[i].code == NTS_SPF_QUAL_L) {
			flag = 1;
			break;
		}
	}

	for(; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		if(is_follow(s, next_tk)) {
			if(flag) {
				if(s->code == NTS_POINTER) {
					stack[top].code = s->code;
					return 0;
				}
			}
			else if(s->code == NTS_UNARY_OPTOR) {
				stack[top].code = s->code;
				return 0;
			}

		}
	}
	return 1;
}

int type_specifier(int i)
{
	struct set *s;
	struct table *tb;
	int nr;
	int code;
	int tmp;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	for(; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		if(is_follow(s, next_tk)) { 
			if(top) {
				tmp = 1; 
				while(tmp < 5) {
					switch(stack[top - tmp].code) {
						case NTS_STRUCT_DECLON_L:
							{						
							if(s->code == NTS_SPF_QUAL_L) {
								stack[top].code = s->code;
								return 0;
							}
							tmp = 5;
							break;
						}
						case NTS_TP_SPF:
							break;
					}
					++tmp;
				}
			} else if(s->code == NTS_DECLON_SPF) {
				stack[top].code = s->code;
				return 0;
			}
		}
	}
	return 1;
}

int specifier_qualifier_list(int i)
{
	struct set *s;
	struct table *tb;
	int nr;
	int code;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	
	for(; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		
		if(is_follow(s, next_tk) &&
				top && stack[top - 1].code == TS_LPAREN &&
				s->code == NTS_TP_NAME) {
			stack[top].code = s->code;
			return 0;
		}
	}
	return 1;
}

int declarator(int i)
{
	struct set *s;
	struct table *tb;
	int j, k;
	int nr;
	int code;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	if(top > 0) {
		for(i = 0; i < nr; ++i) { 
			tb = table + tb_index[code] + i;
			s = set + tb->s_index;
			if(tb->r_index == 0) 
				break;	
			for(j = tb->r_index - 1, k = top - 1; 
					k >= 0 && j >= 0; --j, --k) {
				if(s->right[j] != stack[k].code)
					break; 
			}

			if(j == -1) { 
				switch(stack[top - 1].code) {
					case NTS_DECLON_SPF:
						{
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
						break;}
					case TS_COMMA:
						{	top -= tb->r_index;
						stack[top].code = s->code;
						return 0;}
					case TS_LPAREN:
						return 1;
				}
			}
		}
	}

	for(i = 0; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		if(tb->r_index)
			continue; 
		if(is_follow(s, next_tk) && s->right[1] == 0 && top) {
			int i = 1;
			switch(stack[top - i].code) {
				case TS_COMMA:
					goto next;
			}
		}
	}
	return 1;
}

int exp13(int i)
{
	struct set *s;
	struct table *tb;
	int nr;
	int code;
	int cmp;

	code = stack[top].code;
	nr = tb_index[code + 1] - tb_index[code]; 

	switch(code) {
		case NTS_POSTFIX_EXP:
			{
			cmp = NTS_UNARY_EXP;
			break;}
		case NTS_UNARY_EXP:
			{
			cmp = NTS_CST_EXP;
			break;}
		case NTS_CST_EXP:
			{
				cmp = NTS_MUL_EXP;
			break;}
		case NTS_MUL_EXP:{
			cmp = NTS_ADD_EXP;
			break;}
		case NTS_ADD_EXP:{
			cmp = NTS_SHIFT_EXP;
			break;}
		case NTS_SHIFT_EXP:{
			cmp = NTS_RELAT_EXP;
			break;}
		case NTS_RELAT_EXP:{
			cmp = NTS_EQ_EXP;
			break;}
		case NTS_EQ_EXP:{
			cmp = NTS_BITAND_EXP;
			break;}
		case NTS_BITAND_EXP:{
			cmp = NTS_XOR_EXP;
			break;}
		case NTS_XOR_EXP:{
			cmp = NTS_BITOR_EXP;
			break;}
		case NTS_BITOR_EXP:{
			cmp = NTS_AND_EXP;
			break;}
		case NTS_AND_EXP:{
			cmp = NTS_OR_EXP;
			break;}
		case NTS_OR_EXP:{
			cmp = NTS_COND_EXP;
			break;}
	}
	
	for(; i < nr; ++i) {
		tb = table + tb_index[code] + i;
		s = set + tb->s_index;
		
		if(is_follow(s, next_tk)) { 
			if(s->code == cmp) {
				stack[top].code = s->code;
				return 0;
			}
		}
	}
	return 1;
}
*/
int (*reduction[])(int i) = {
	[33] = mul_star, 
	[68] = r_parenthesis,
	[73] = id,
//	[76] = exp13, //bit_AND_exp,
//	[78] = exp13, //add_exp,
	[80] = assignment_exp,
//	[87] = exp13, //cast_exp,
	[89] = conditional_exp,
	[91] = declaration,
	[94] = declarator,
//	[99] = direct_declarator,
	[130] = specifier_qualifier_list,
	[138] = type_specifier,
//	[140] = exp13, //unary_exp,
};

//0-归约成功继续下一次归约，1-无归约移入新tk，-1-错误
int main_reduction(int code)
{
	struct set *s;
	struct table *tb;
	int i, j;
	int tmp;
	int nr;
	int gn = 0;

	if(IS_DIRECT_IN(code))
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
				switch(code) {
					case NTS_DIRECT_DECLOR:
						if(next_tk == TS_LBRAKET || next_tk == TS_LPAREN) {
							return 1;
						}
					case NTS_STM:
						if(next_tk == TS_ELSE)
							return 1;
					case NTS_ASSIGNMENT_EXP:
						{
						if(stack[top - 2].code == NTS_POSTFIX_EXP)
							gn = 1;
						break;
						}
				}
				if(s->right[tb->r_index + 1] == 0) {
					top -= tb->r_index;
					stack[top].code = s->code;
					return 0;
				}
				if(!gn)
					return 1; 
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
		if(is_follow(s, next_tk)) {
			stack[top].code = s->code;
			return 0;
		}
		else
			return 1;
	}

	return reduction[stack[top].code](i);
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
	//	printf("\ncurr: %s   next: %s\n", 
	//			token_str[code], token_str[next_tk]);

		while(over == 0) {
			code = stack[top].code;
			switch(code) {
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
	}
	return -1;
}

int main(int argc, char *argv[]) 
{
	init_tb_index();
	init_ff();
	read_source(argv[1]);
	//printf("ret: %d\n", LR());
	close_source();
	return 0;
}
