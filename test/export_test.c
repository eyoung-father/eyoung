#include <stdio.h>
#include "ey_export.h"

int a=1;

int foo(void *link, void *event)
{
	printf("call foo, a=%d\n", ++a);
	return 1;
}

int bar(void *link, void *event)
{
	printf("call bar, a=%d\n", ++a);
	return 0;
}

static void test_init()
{
	printf("call init, a=%d\n", a);
}

static void test_exit()
{
	printf("call finit, a=%d\n", a);
}

struct s
{
	int a;
	int b;
};

EY_EXPORT_IDENT(a, "int a;");
EY_EXPORT_IDENT(foo, "int foo();");
EY_EXPORT_TYPE(s, "struct s{int a; int b;};");

EY_EXPORT_INIT(test_init);
EY_EXPORT_FINIT(test_exit);
