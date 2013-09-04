#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "ey_info.h"

int debug_engine_init = 1;
int debug_engine_lexier = 1;

int engine_parser_error(const char *format, ...)
{
	if(!debug_engine_parser )
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int engine_parser_debug(const char *format, ...)
{
	if(!debug_engine_parser)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int engine_lexer_error(const char *format, ...)
{
	if(!debug_engine_lexier)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int engine_lexer_debug(const char *format, ...)
{
	if(!debug_engine_lexier)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int engine_init_error(const char *format, ...)
{
	if(!debug_engine_init)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}

int engine_init_debug(const char *format, ...)
{
	if(!debug_engine_init)
		return 0;
	
	va_list ap;
	int ret = 0;
	va_start(ap, format);
	ret = vfprintf(stderr, format, ap);
	va_end(ap);
	return ret;
}
