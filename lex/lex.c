#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <malloc.h> 
#include "../icc.h"

extern struct input input;
extern int value;
extern char *token_str[SYM_SIZE];

extern int no;

static inline char fgetch(FILE *fp)
{
	char ch;
	ch = fgetc(fp);
	if(ch == '\n')
		++no;
	return ch;
}

static inline void ungetch(char ch, FILE *fp)
{
	if(ch == '\n')
		--no;
	ungetc(ch, fp);
}

void read_source(char *filename)
{
	FILE *fp;

	input.filename = filename;
	fp = fopen(filename, "r");
	if(fp == NULL) {
		perror("error:");
		exit(0);
	}
	input.fp = fp;
	input.ch = fgetch(fp);
}

void close_source(void)
{
	fclose(input.fp);
}


static int hex(char ch)
{
	if ((ch >= '0') && (ch <= '9'))
		return ch - '0';
	if ((ch >= 'a') && (ch <= 'f'))
		return ch - 'a' + 10;
	if ((ch >= 'A') && (ch <= 'F'))
		return ch - 'A' + 10;
	return -1;
}

int digit_lex(void)
{
	int val= 0;
	char ch;
	FILE *fp;
	int ret;

	fp = input.fp;
	ch = input.ch;
	if(ch == '0') {
		ch = fgetch(fp);
		if(isdigit(ch)) {	//八进制
			do {
				val *= 8;
				val += ch - '0';
			}while(isdigit(ch = fgetch(fp)));
		} else if(ch == 'x' || ch == 'X') { //16进制
			ch = fgetch(fp);
			if(isalnum(ch)) {
				do {
					ret = hex(ch);
					if(ret == -1) {
						return ERR_HEX;
					}
					val *= 16;
					val += ret;
				}while(isalnum(ch = fgetch(fp)));
			}
		}
	}
	else { //十进制
		do {
			val *= 10;
			val += ch - '0';
		}while(isdigit(ch = fgetch(fp)));
	}

	input.ch = ch;
	value = val;

	if(ispunct(ch) || isspace(ch)) {
		return TS_CONSTANT;
	}
	return ERR_ID;
}

int alpha_lex(void)
{
	char ch;
	FILE *fp;
	int i;
	int ret = TS_ID;
	char word[256] = {0};

	word[0] = input.ch;
	fp = input.fp;
	i = 1;
	ch = fgetch(fp);
	while((isalnum(ch) || ch == '_') && i < 256) {
		word[i++] = ch;
		ch = fgetch(fp);

	}
	if(ch == '\"' || ch == '\'' ||
			ch == '@'|| ch == '#' ||
			ch == '$'|| ch == '`')
		return ERR_ID;

	if(i >= 256)
		return ERR_ID_FLOW;
	word[i] = 0;
	input.ch = ch;

	if(i > 1 && i < 10) {
		switch(word[0]) {
			case 'b':
				if(strncmp(word, "break", 6) == 0)
					ret = TS_BREAK;
				break;
			case 'c':
				if(strncmp(word, "char", 5) == 0)
					ret = TS_CHAR;
				else if(strncmp(word, "case", 5) == 0)
					ret = TS_CASE;
				//else if(strncmp(word, "const", 6) == 0)
				//	ret = TS_CONST;
				else if(strncmp(word, "continue", 9) == 0)
					ret = TS_CONTINUE;
				break;
			case 'd':
				if(strncmp(word, "do", 3) == 0)
					ret = TS_DO;
				else if(strncmp(word, "default", 8) == 0)
					ret = TS_DEFAULT;
				break;
			case 'e':
				if(strncmp(word, "enum", 5) == 0)
					ret = TS_ENUM;
				else if(strncmp(word, "else", 5) == 0)
					ret = TS_ELSE;
				else if(strncmp(word, "extern", 7) == 0)
					ret = TS_EXTERN;
				break;
			case 'f':
				if(strncmp(word, "for", 4) == 0)
					ret = TS_FOR;
				break;
			case 'g':
				if(strncmp(word, "goto", 5) == 0)
					ret = TS_GOTO;
				break;
			case 'i':
				if(strncmp(word, "if", 3) == 0)
					ret = TS_IF;
				else if(strncmp(word, "int", 4) == 0)
					ret = TS_INT;
				//else if(strncmp(word, "inline", 7) == 0)
				//	ret = TS_INLINE;
				break;
			case 'l':
				if(strncmp(word, "long", 5) == 0)
					ret = TS_LONG;
				break;
			case 'r':
				if(strncmp(word, "return", 7) == 0)
					ret = TS_RETURN;
				break;
			case 's':
				if(strncmp(word, "signed", 7) == 0)
					ret = TS_SIGNED;
				else if(strncmp(word, "short", 6) == 0)
					ret = TS_SHORT;
				else if(strncmp(word, "static", 7) == 0)
					ret = TS_STATIC;
				else if(strncmp(word, "switch", 7) == 0)
					ret = TS_SWITCH;
				else if(strncmp(word, "sizeof", 7) == 0)
					ret = TS_SIZEOF;
				else if(strncmp(word, "struct", 7) == 0)
					ret = TS_STRUCT;
				break;
			case 't':
				if(strncmp(word, "typedef", 8) == 0)
					ret = TS_TYPEDEF;
				break;
			case 'u':
				if(strncmp(word, "union", 6) == 0)
					ret = TS_UNION;
				else if(strncmp(word, "unsigned", 9) == 0)
					ret = TS_UNSIGNED;
				break;
			case 'v':
				if(strncmp(word, "void", 5) == 0)
					ret = TS_VOID;
			//	else if(strncmp(word, "volatile", 9) == 0)
			//		ret = TS_VOLATILE;
				break;
			case 'w':
				if(strncmp(word, "while", 6) == 0)
					ret = TS_WHILE;
				break;
			default:
				ret = TS_ID;
		}
	}
/*
	if(ret == TS_ID) {
		char *str;
		str = malloc(i + 1);
		if(str == NULL)
			return ERR_MEM;
		strncpy(str, word, i);	
		str[i] = 0;
		token_str[ret] = str;
	}
*/
	return ret;
}

int alpha_lex_2(void)
{
	char ch;
	FILE *fp;
	int i;
	int ret = TS_ID;
	char word[256];

	word[0] = input.ch;
	fp = input.fp;
	i = 1;
	ch = fgetch(fp);
	while((isalnum(ch) || ch == '_') && i < 256) {
		word[i++] = ch;
		ch = fgetch(fp);
	}

	if(ch == '\"' || ch == '\'' ||
			ch == '@'|| ch == '#' ||
			ch == '$'|| ch == '`')
		return ERR_ID;

	if(i >= 256)
		return ERR_ID_FLOW;
	word[i] = 0;
	input.ch = ch;

	if(i > 1 && i < 10)
		switch(word[0]) {
			case 'b':
				if(i == 5 && strncmp(word + 1, "reak", 4) == 0)
					ret = TS_BREAK;
				break;
			case 'c':
				if(i == 4 && strncmp(word + 1, "har", 3) == 0)
					ret = TS_CHAR;
				else if(i == 4 && strncmp(word + 1, "ase", 3) == 0)
					ret = TS_CASE;
				//if(i == 5 && strncmp(word + 1, "onst", 4) == 0)
				//	ret = TS_CONST;
				else if(i == 8 && strncmp(word + 1, "ontinue", 7) == 0)
					ret = TS_CONTINUE;
				break;
			case 'd':
				if(i == 2 && word[1] == 'o')
					ret = TS_DO;
				else if(i == 7 && strncmp(word + 1, "efault", 6) == 0)
					ret = TS_DEFAULT;
				break;
			case 'e':
				if(i == 4 && strncmp(word + 1, "num", 3) == 0)
					ret = TS_ENUM;
				else if(i == 4 && strncmp(word + 1, "lse", 3) == 0)
					ret = TS_ELSE;
				else if(i == 6 && strncmp(word + 1, "xtern",5) == 0)
					ret = TS_EXTERN;
				break;
			case 'f':
				if(i == 3 && strncmp(word + 1, "or", 2) == 0)
					ret = TS_FOR;
				break;
			case 'g':
				if(i == 4 && strncmp(word + 1, "oto", 3) == 0)
					ret = TS_GOTO;
				break;
			case 'i':
				if(i == 2 && word[1] == 'f')
					ret = TS_IF;
				else if(i == 3 && strncmp(word + 1, "nt", 2) == 0)
					ret = TS_INT;
				//	else if(i == 6 && strncmp(word + 1, "nline", 5) == 0)
				//		ret = TS_INLINE;
				break;
			case 'l':
				if(i == 4 && strncmp(word + 1, "ong", 3) == 0)
					ret = TS_LONG;
				break;
			case 'r':
				if(i == 6 && strncmp(word + 1, "eturn", 5) == 0)
					ret = TS_RETURN;
				break;
			case 's':
				switch(word[1]){
					case 'i':
						if(i == 6 && strncmp(word + 2, "gned", 5) == 0)
							ret = TS_SIGNED;
						else if(i == 6 && strncmp(word + 2, "zeof", 5) == 0)
							ret = TS_SIZEOF;
						break;
					case 'h':
						if(i == 5 && strncmp(word + 2, "ort", 4) == 0)
							ret = TS_SHORT;
						break;
					case 't':

						if(i == 6 && strncmp(word + 2, "atic", 5) == 0)
							ret = TS_STATIC;
						else if(i == 6 && strncmp(word + 2, "ruct", 5) == 0)
							ret = TS_STRUCT;
						break;
					case 'w':
						if(i == 6 && strncmp(word + 2, "itch", 5) == 0)
							ret = TS_SWITCH;
						break;
				}
				break;
			case 't':
				if(i == 7 && strncmp(word + 1, "ypedef", 6) == 0)
					ret = TS_TYPEDEF;
				break;
			case 'u':
				if(i == 5 && strncmp(word + 1, "nion", 4) == 0)
					ret = TS_UNION;
				else if(i == 8 && strncmp(word + 1, "nsigned", 7) == 0)
					ret = TS_UNSIGNED;
				break;
			case 'v':
				if(i == 4 && strncmp(word + 1, "oid", 3) == 0)
					ret = TS_VOID;
				//else if(i == 8 && strncmp(word + 1, "olatile", 7) == 0)
				//	ret = TS_VOLATILE;
				break;
			case 'w':
				if(i == 5 && strncmp(word + 1, "hile", 4) == 0)
					ret = TS_WHILE;
				break;
			default:
				ret = TS_ID;
		}
/*
	if(ret == TS_ID) {
		char *str;
		str = malloc(i + 1);
		if(str == NULL)
			return ERR_MEM;
		strncpy(str, word, i);	
		str[i] = 0;
		token_str[ret] = str;
	}
*/
	return ret;
}

int skip_comment(void)
{
	char ch;
	FILE *fp;
	char line[256];

	fp = input.fp;
	ch = input.ch;
	if(ch == '/') {
		fgets(line, 256, fp);
		++no;
		return 0;
	}
	
	if(ch == '*') {
		do{
			ch = fgetch(fp);
			if(ch == '/' && (ch = fgetch(fp) == '*'))
				return ERR_COMMENT;
		} while(!(ch == '*' && (ch = fgetch(fp)) == '/') && ch != EOF);

		if(ch == EOF)
			return ERR_COMMENT;
	}
	return 0;
}

int escap_lex(void)
{
	char ch, next_ch;
	FILE *fp;

	fp = input.fp;
	ch = fgetch(fp);
//	printf("now: %c\n", ch);
	switch(ch) {
		case 'a':
			return '\a';
		case 'b':
			return '\b';
		case 'f':
			return '\f';
		case 'n':
			return '\n';
		case 'r':
			return '\r';
		case 't':
			return '\t';
		case 'v':
			return '\v';
		case '\\':
			return '\\';
		case '\'':
			return '\'';
		case '\"':
			return '\"';
		case 'x':
		case 'X':
			ungetch(ch, fp);
			input.ch = 0;
			digit_lex();
			ungetch(input.ch, fp);
			//	printf("eacap value:%d\n", value);
			return value;

		default:
			if(isdigit(ch)) {
				input.ch = ch;
				digit_lex();
				ungetch(input.ch, fp);
				//	printf("eacap value:%d\n", value);
				return value;
			}
			else
				return ERR_ESCAP;
	}
}

int string_lex(char ch)
{
	char next_ch;
	FILE *fp;
	int ret;
	int i;
	char *str;

	fp = input.fp;
	next_ch = input.ch;
	switch(ch) {
		case '\\':
			{
				char line[256];
				memset(line, 0, 256);
				str = line;
				fgets(str, 256, fp);
				++no;
				while(*str) {
					if(!isspace(*str++))
						return ERR_ESCAP;
				}
			}
			break;
		case '\"':
			str = malloc(256);
			if(str == NULL)
				return ERR_MEM;
			memset(str, 0, 256);
			i = 0;
			while(next_ch != '\"' && i < 256) {
				if(next_ch == '\'' || next_ch == EOF) {
					free(str);
					input.ch = next_ch; 
					return ERR_STRING;
				}
				if(next_ch == '\\') {
					str[i++] = escap_lex();
				}
				else
					str[i++] = next_ch;
				next_ch = fgetch(fp);
			}
			token_str[TS_STR] = str;
				//printf(" str:%s\n", str);
			ret = TS_STR;
			break;
		case '\'':
			if(next_ch == '\'') {
				value = 0;
				//ret = TS_CCONST;
				ret = TS_CONSTANT;
				break;
			}
			if(next_ch == '\\')
				value = escap_lex();
			else 
				value = next_ch;
			while((next_ch = fgetch(fp)) != '\'' && next_ch != EOF);
			if(next_ch == EOF) {
				input.ch = next_ch;
				return ERR_CONST_CHAR;
			}
			//ret = TS_CCONST;
			ret = TS_CONSTANT;
			break;
	}
	
	return ret;
}


int punct_lex(void)
{
	char ch, next_ch;
	FILE *fp;
	int ret;

	fp = input.fp;
	ch = input.ch;
	next_ch = fgetch(fp);

	switch(ch) {
		case '+':
			if(next_ch == '+')
				ret = TS_INC;
			else if(next_ch == '=')
				ret = TS_ADD_ASSIGN;
			else
				ret = TS_ADD;
			break;
		case '-':
			if(next_ch == '-')
				ret = TS_DEC;
			else if(next_ch == '=')
				ret = TS_SUB_ASSIGN;
			else if(next_ch == '>')
				ret = TS_POINTER;
			else
				ret = TS_SUB;
			break;
		case '*':
			if(next_ch == '=')
				ret = TS_MUL_ASSIGN;
			else
				ret = TS_MUL;
			break;
		case '/':
			if(next_ch == '=')
				ret = TS_DIV_ASSIGN;
			else if(next_ch == '/' || next_ch == '*') {
				input.ch = next_ch;
				if(skip_comment() < 0)
					return ERR_COMMENT;
			//	printf("\nbefore comment:%c\n", next_ch);
				input.ch = fgetch(fp);
				return get_next_token();
			}
			else
				ret = TS_DIV;
			break;
		case '%':
			if(next_ch == '=')
				ret = TS_MOD_ASSIGN;
			else
				ret = TS_MOD;
			break;
		case '<':
			if(next_ch == '=')
				ret = TS_LESS_EQ;
			else if(next_ch == '<') {
				next_ch = fgetch(fp);
				if(next_ch == '=')
					ret = TS_LSHIFT_ASSIGN;
				else
				{
					ungetch(next_ch, fp);
					ret = TS_LSHIFT;
				}
			}
			else
				ret = TS_LESS;
			break;
		case '>':
			if(next_ch == '=')
				ret = TS_GREATER_EQ;
			else if(next_ch == '>') {
				next_ch = fgetch(fp);
				if(next_ch == '=')
					ret = TS_RSHIFT_ASSIGN;
				else 
				{
					ungetch(next_ch, fp);
					ret = TS_RSHIFT;
				}
			}
			else
				ret = TS_GREATER;
			break;
		case '&':
			if(next_ch == '=')
				ret = TS_BITAND_ASSIGN;
			else if(next_ch == '&')
				ret = TS_AND;
			else
				ret = TS_BITAND;
			break;
		case '|':
			if(next_ch == '=')
				ret = TS_BITOR_ASSIGN;
			else if(next_ch == '|')
				ret = TS_OR;
			else
				ret = TS_BITOR;
			break;
		case '=':
			if(next_ch == '=')
				ret = TS_EQ;
			else
				ret = TS_ASSIGN;
			break;
		case '!':
			if(next_ch == '=')
				ret = TS_NOT_EQ;
			else
				ret = TS_NOT;
			break;
		case '^':
			if(next_ch == '=')
				ret = TS_BITXOR_ASSIGN;
			else
				ret = TS_BITXOR;
			break;
		case '{':
			ret = TS_LBRACE;
			break;
		case '}':
			ret = TS_RBRACE;
			break;
		case '(':
			ret = TS_LPAREN;
			break;
		case ')':
			ret = TS_RPAREN;
			break;
		case '[':
			ret = TS_LBRAKET;
			break;
		case ']':
			ret = TS_RBRAKET;
			break;
		case '.':
			if(next_ch == '.') {
				next_ch = fgetch(fp);
				if(next_ch == '.')
					ret = TS_ELLIPSE;
				else {
					ungetch(next_ch, fp);
					ret = ERR_ELLIPSE;
				}
			}
			else
				ret = TS_DOT;
			break;

		case '?':
			ret = TS_QUESTION;
			break;
		case '~':
			ret = TS_COMPL;
			break;
		case ':':
			ret = TS_COLON;
			break;
		case ';':
			ret = TS_SEMICOLON;
			//printf("%c\n", next_ch);
			break;
		case ',':
			ret = TS_COMMA;
			break;
		case '#':
			//预编译范畴
			break;
		case '`':
		case '@':
		case '$':
			return ERR_ID;

		default: //  \ " ' 
			input.ch = next_ch;
			ret = string_lex(ch);
			next_ch = fgetch(fp);
	}

	switch(next_ch) {
		case '+':
		case '-':
		case '=':
		case '>':
		case '<':
		case '&':
		case '|':
			input.ch = fgetch(fp);
			break;
		default:
			input.ch = next_ch;
	}
			
	return ret;
}

int get_next_token(void)
{
	FILE *fp;
	char ch;
	/*static int nr = 0;
	printf("times: %d\n", nr++);
*/
	fp = input.fp;
	ch = input.ch;
	while(isspace(ch)){
		ch = fgetch(fp);
	}
	input.ch = ch;
	if(isalpha(ch) || ch == '_')
		return alpha_lex_2(); 
	if(ispunct(ch))
		return punct_lex();
	if(isdigit(ch))
		return digit_lex();
	return END_OF_FILE;
}

void test(void)
{
	int token;
	int nr = 0;

	while(input.ch != EOF) {
	//	printf("now ch:%c\n", input.ch);
		token = get_next_token();
	//	printf("now ch:%c\n", input.ch);
	//	printf("%d\n", token);
		if(token > 0 && token < 80) {
			if(token == TS_CONSTANT)
				printf("%d ", value);
			else
				printf("%s ", token_str[token]);
			if(token == TS_ID || token == TS_STR)
				free(token_str[token]);
		}
		else {
			printf("err %d at line %d\n", token, no);
			break;
		}
	}
}

