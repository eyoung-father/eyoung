#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "http.h"
#include "http_private.h"
#include "http_client_lex.h"
#include "http_server_lex.h"

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

size_t http_parse_integer(const char *str, int mode, int *error)
{
	char *end = NULL;
	errno = 0;
	long ret = strtol(str, &end, mode);
	*error = 0;

	if(errno)
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

int http_parse_chunk_header(http_decoder_t *decoder, http_string_t *line, http_chunk_body_header_t *header, int from_client)
{
	int need_debug = (from_client?debug_http_client_lexer:debug_http_server_lexer);
	char *end = NULL;
	if(!line || !line->len || !line->buf || !header)
	{
		http_debug(need_debug, "bad paramter\n");
		return -1;
	}
	
	errno = 0;
	header->chunk_size = strtol(line->buf, &end, 16);
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
	
	header->chunk_extension.buf = NULL;
	header->chunk_extension.len = 0;
	if(*end)
	{
		int chunk_ext_len = line->len - (end-line->buf);
		if(!http_alloc_string(decoder, end, chunk_ext_len, &header->chunk_extension, from_client))
		{
			http_debug(need_debug, "alloc chunk extension failed\n");
			return -1;
		}
	}

	return 0;
}

int http_prepare_string(http_decoder_t *decoder, char *line, size_t length, http_string_t *string, int no_newline, int from_client)
{
	int need_debug = (from_client?debug_http_client_lexer:debug_http_server_lexer);
	if(!string || !line || !length)
	{
		http_debug(need_debug, "bad parameter in %s\n", __FUNCTION__);
		return -1;
	}
	
	string->buf = line;
	string->len = length;
	
	if(no_newline)
	{
		if(line[length-1] != '\n')
		{
			http_debug(need_debug, "last char is not \\n\n");
			return -1;
		}
		length--;

		if(length && line[length-1] == '\r')
			length--;

		string->len = length;
	}

	return 0;
}

void http_lexer_set_length(void *scanner, size_t length, int from_client)
{
	yyscan_t yyscanner = (yyscan_t)scanner;
	http_data_t *priv = NULL;
	http_parser_t *parser = NULL;
	if(from_client)
	{
		priv = (http_data_t*)http_client_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->request_parser;
		assert(parser != NULL);
	}
	else
	{
		priv = (http_data_t*)http_server_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->response_parser;
		assert(parser != NULL);
	}
	
	parser->length = length;
}

size_t http_lexer_get_length(void *scanner, int from_client)
{
	yyscan_t yyscanner = (yyscan_t)scanner;
	http_data_t *priv = NULL;
	http_parser_t *parser = NULL;
	if(from_client)
	{
		priv = (http_data_t*)http_client_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->request_parser;
		assert(parser != NULL);
	}
	else
	{
		priv = (http_data_t*)http_server_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->response_parser;
		assert(parser != NULL);
	}
	
	return parser->length;
}

int http_lexer_is_chunked_body(void *scanner, int from_client)
{
	yyscan_t yyscanner = (yyscan_t)scanner;
	http_data_t *priv = NULL;
	http_parser_t *parser = NULL;
	if(from_client)
	{
		priv = (http_data_t*)http_client_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->request_parser;
		assert(parser != NULL);
	}
	else
	{
		priv = (http_data_t*)http_server_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->response_parser;
		assert(parser != NULL);
	}
	
	return parser->chunked != 0;
}

void http_lexer_init_body_info(void *scanner, int from_client)
{
	yyscan_t yyscanner = (yyscan_t)scanner;
	http_data_t *priv = NULL;
	http_parser_t *parser = NULL;
	if(from_client)
	{
		priv = (http_data_t*)http_client_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->request_parser;
		assert(parser != NULL);
	}
	else
	{
		priv = (http_data_t*)http_server_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->response_parser;
		assert(parser != NULL);
	}
	
	parser->length = 0;
	parser->chunked = 0;
}

void http_lexer_set_chunked_body(void *scanner, int from_client)
{
	yyscan_t yyscanner = (yyscan_t)scanner;
	http_data_t *priv = NULL;
	http_parser_t *parser = NULL;
	if(from_client)
	{
		priv = (http_data_t*)http_client_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->request_parser;
		assert(parser != NULL);
	}
	else
	{
		priv = (http_data_t*)http_server_get_extra(yyscanner);
		assert(priv != NULL);

		parser = &priv->response_parser;
		assert(parser != NULL);
	}
	
	parser->chunked = 1;
}
