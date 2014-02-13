#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "http.h"
#include "http_private.h"

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

char* http_string_trim(const char *old_string, size_t old_len, size_t *new_len)
{
	if(!old_string || !old_len || !new_len)
		return NULL;
	
	const char *head = NULL, *tail = NULL;
	for(tail = old_string + old_len - 1; isspace(*tail) && tail > old_string ; tail--);
	for(head = old_string ; isspace(*head) && head<tail; head++);

	*new_len = tail - head;
	if(!isspace(*tail))
		(*new_len)++;
	char *ret = (char*)http_malloc(*new_len+1);
	if(!ret)
	{
		*new_len = 0;
		return NULL;
	}

	if(*new_len)
		memcpy(ret, head, *new_len);
	ret[*new_len] = '\0';
	return ret;
}

int http_parse_chunk_header(http_decoder_t *decoder, const char *line, size_t line_size, size_t *chunk_size, http_string_t *chunk_ext, int from_client)
{
	int need_debug = (from_client?debug_http_client_lexer:debug_http_server_lexer);
	if(!line || !chunk_size || !chunk_ext || line_size<2)
	{
		http_debug(need_debug, "bad paramter\n");
		return -1;
	}

	char *end = NULL;
	errno = 0;
	*chunk_size = strtol(line, &end, 16);
	if(errno)
	{
		http_debug(need_debug, "bad chunk size format\n");
		return -1;
	}

	if(!isspace(*end))
	{
		http_debug(need_debug, "chunk size not followed by space char\n");
		return -1;
	}

	while(isspace(*end))
		end++;
	
	chunk_ext->buf = NULL;
	chunk_ext->len = 0;
	if(*end)
	{
		int chunk_ext_len = line_size - (end-line);
		if(line[line_size-2] == '\r')
			chunk_ext_len -= 1;
		if(!http_alloc_string(decoder, end, chunk_ext_len, chunk_ext, from_client))
		{
			http_debug(need_debug, "alloc chunk extension failed\n");
			return -1;
		}
	}

	return 0;
}
