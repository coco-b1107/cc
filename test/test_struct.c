int a;
unsigned int a[10];
unsigned int a[2][3];
unsigned int a[2][3][4];
unsigned int *a[1];
char **str[2];

unsigned int a;
short s;
struct A;
union B;

static int *pointer;
signed char *str;
char **str;
long l, *p;
enum tag{
	FIRST,
	FOLLOW,
	LAST,
	TEST
};
struct token{
	int a, b, c, d, e, f, g;
	short s;
	char ch;
	signed int si;
	unsigned int ui;
	long *c;
	struct A a;
	union {
		int a;
		char ch;
	} a;
};

union type{
	int a;
	unsigned char ch;
};
