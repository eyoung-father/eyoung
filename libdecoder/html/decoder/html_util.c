#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "html.h"
#include "html_private.h"

int debug_html_lexer=1;
int debug_html_parser=1;
int debug_html_detect=1;
int debug_html_mem=1;

int html_debug(int flag, char *format, ...)
{
	if(!flag)
		return 0;

	va_list ap;
	int ret = 0;

	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int html_abnormal(int flag, char *format, ...)
{
	if(!flag)
		return 0;

	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int html_attack(int flag, char *format, ...)
{
	if(!flag)
		return 0;

	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int html_error(void *this_priv, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1] = '\0';
	html_debug(debug_html_lexer||debug_html_parser||debug_html_detect, "%s\n", buffer);
	va_end(args);
	return 0;
}
