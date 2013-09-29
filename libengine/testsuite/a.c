%{
#include <stdio.h>
#include <stdlib.h>

#define MY_MACRO(eng)               \
	do                              \
	{                               \
		printf("test:%d\n", (eng)); \
	}while(0)
static int my_printf(const char *fmt, ...);
%}

%import "libtest.so"
%output "test.ey"
%event "http_cookie" "void"
%event "http_referer" "void"
%event "http_uri" "void"
%init "ac_init"
%finit "ac_finit"

%%
1:http_uri(foo(_LINK_, _THIS_))/"testest"
	{
		my_printf("uri foo()\n");
		return 1;
	}
	;

2:http_cookie(foo(_LINK_, /*ct test*/_THIS_)) /*ct test*/
	{
		my_printf("cookie foo()\n");
		return 1;
	}
	http_referer(foo(_LINK_, _THIS_)/*ct test*/)
	{/*ct test*/
		my_printf("referer foo()\n");
		return 1;
	}
	| http_referer(bar(_LINK_, _THIS_))
	{
		my_printf("referer bar()\n");
		return 1;
	}http_cookie(bar(_LINK_, _THIS_))/"testest"
	{
		my_printf("cookie bar()\n");
		return 1;
	}
	;
%%
static int my_printf(const char *fmt, ...)
{
	return fprintf(stderr, /*testtest*/"%s\n", fmt);
}

int ac_init(void *eng)
{
	fprintf(stderr, "ac_init return 0\n");
	return 0;
}

int ac_finit(void *eng)
{
	fprintf(stderr, "ac_finit return 0\n");
	return 0;
}
