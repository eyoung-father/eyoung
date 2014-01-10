#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "http_util.h"

int debug_http_mem=1;
int debug_http_client_lexer=1;
int debug_http_server_lexer=1;
int debug_http_client_parser=1;
int debug_http_server_parser=1;
int debug_http_detect=1;
int http_debug(int flag, char *format, ...)
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

int http_abnormal(int flag, char *format, ...)
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

int http_attack(int flag, char *format, ...)
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

int http_parse_integer(char *str, int *error)
{
	char *end = NULL;
	errno = 0;
	long ret = strtol(str, &end, 10);
	*error = 0;

	if(end)
	{
		*error = EINVAL;
		return ret;
	}

	if(!ret)
	{
		*error = errno;
		return ret;
	}

	/*integer overflow?*/
	return ret;
}

int http_server_error(void *this_priv, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1] = '\0';
	http_debug(debug_http_server_lexer||debug_http_server_parser||debug_http_detect, "%s\n", buffer);
	va_end(args);
	return 0;
}

int http_client_error(void *this_priv, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1] = '\0';
	http_debug(debug_http_client_lexer||debug_http_client_parser||debug_http_detect, "%s\n", buffer);
	va_end(args);
	return 0;
}
