#include "ey_export.h"

int a;
int foo()
{
	return 0;
}

static void test_init()
{

}

static void test_exit()
{

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
