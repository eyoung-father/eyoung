#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "snort_info.h"

int debug_snort_init = 1;
int debug_snort_lexer = 1;

int snort_parser_error(const char *format, ...)
{
	if(!debug_snort_parser )
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int snort_parser_debug(const char *format, ...)
{
	if(!debug_snort_parser)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int snort_lexer_error(const char *format, ...)
{
	if(!debug_snort_lexer)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int snort_lexer_debug(const char *format, ...)
{
	if(!debug_snort_lexer)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int snort_init_error(const char *format, ...)
{
	if(!debug_snort_init)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int snort_init_debug(const char *format, ...)
{
	if(!debug_snort_init)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}
