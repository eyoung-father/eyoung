%{
#include <stdio.h>
#include "tset.h"

int test=10;
static int /*testest*/my_printf(const char /*testtes*/*fmt, ...);
#define MY_MACRO(eng)               \
	do                              \
	{                               \
		printf("test:%d\n", (eng)); \
	}while(0)
%}

%import "libtest.so"
%output "test.cxx"

%%
1:http_uri(aaa(/*ttt*/))/"testest"
	;

2:http_cookie(aaa()) http_referer(bbb())
	| http_referer(bbb) http_cookie(aaa())/"testest"
	;
%%
static int my_printf(const char *fmt, ...)
{
		return fprintf(stderr, /*testtest*/"%s\n", fmt);
}
