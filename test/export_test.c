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

int test_init(void *eng)
{
	printf("call init, a=%d\n", a++);
}

int test_exit(void *eng)
{
	printf("call finit, a=%d\n", a--);
}

struct s
{
	int a;
	int b;
};

EY_EXPORT_IDENT(a, "extern int a;");
EY_EXPORT_IDENT(foo, "int foo(void *link, void *event);");
EY_EXPORT_IDENT(bar, "int bar(void *link, void *event);");
EY_EXPORT_TYPE(s, "struct s{int a; int b;};");

EY_EXPORT_INIT(test_init);
EY_EXPORT_FINIT(test_exit);
