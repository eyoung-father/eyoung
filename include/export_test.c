#ifdef EY_EXPORT_TEST

#include "ey_export.h"

int a;
int foo()
{
	return 0;
}

void test_init()
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

void main()
{
}

#endif
