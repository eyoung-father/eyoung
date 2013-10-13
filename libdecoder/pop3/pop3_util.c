#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "pop3_util.h"

int debug_pop3_mem=1;
int debug_pop3_client=1;
int debug_pop3_server=1;
int debug_pop3_ips=1;
int pop3_debug(int flag, char *format, ...)
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

int pop3_abnormal(int flag, char *format, ...)
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

int pop3_attack(int flag, char *format, ...)
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

int pop3_parse_integer(char *str, int *error)
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

int pop3_server_error(void *this_priv, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1] = '\0';
	pop3_debug(debug_pop3_server, "%s\n", buffer);
	va_end(args);
	return 0;
}

int pop3_client_error(void *this_priv, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer)-1, format, args);
	buffer[sizeof(buffer)-1] = '\0';
	pop3_debug(debug_pop3_client, "%s\n", buffer);
	va_end(args);
	return 0;
}
