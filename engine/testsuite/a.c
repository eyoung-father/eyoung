%{
#include <stdio.h>
#include <stdlib.h>

int test=10;
extern int aaa(void);
extern int bbb(void);
static int /*testest*/my_printf(const char /*testtes*/*fmt, ...);
#define MY_MACRO(eng)               \
	do                              \
	{                               \
		printf("test:%d\n", (eng)); \
	}while(0)
%}

%import "libtest.so"
%output "test.cxx"
%event "http_cookie" "void*"
%event "http_referer" "void*"
%event "http_uri" "void*"

%%
1:http_uri(aaa(/*ttt*/))/"testest"
	;

2:http_cookie(aaa()) http_referer(bbb())
	| http_referer(bbb()) http_cookie(aaa())/"testest"
	;
%%
static int my_printf(const char *fmt, ...)
{
		return fprintf(stderr, /*testtest*/"%s\n", fmt);
}
