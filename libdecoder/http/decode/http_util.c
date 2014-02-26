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

void http_lexer_set_content_encoding(void *scanner, http_body_content_encoding_t encoding, int from_client)
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
	
	parser->content_encoding = encoding;
}

http_body_content_encoding_t http_lexer_get_content_encoding(void *scanner, int from_client)
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
	
	return parser->content_encoding;
}

void http_lexer_set_content_language(void *scanner, http_body_content_language_t language, int from_client)
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
	
	parser->content_language = language;
}

http_body_content_language_t http_lexer_get_content_language(void *scanner, int from_client)
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
	
	return parser->content_language;
}

void http_lexer_set_content_type(void *scanner, http_body_content_maintype_t main_type, 
									http_body_content_subtype_t sub_type, 
									http_body_content_charset_t charset, int from_client)
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
	
	parser->content_maintype = main_type;
	parser->content_subtype = sub_type;
	parser->content_charset = charset;
}

void http_lexer_get_content_type(void *scanner, http_body_content_maintype_t *main_type, 
									http_body_content_subtype_t *sub_type, 
									http_body_content_charset_t *charset, int from_client)
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
	
	*main_type = parser->content_maintype;
	*sub_type = parser->content_subtype;
	*charset = parser->content_charset;
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

http_body_content_encoding_t http_parse_content_encoding(const char *value)
{
	if(!value)
		return HTTP_BODY_CONTENT_ENCODING_UNKOWN;

	if(!strncasecmp(value, "gzip", sizeof("gzip")-1))
		return HTTP_BODY_CONTENT_ENCODING_GZIP;
	else if(!strncasecmp(value, "deflate", sizeof("deflate")-1))
		return HTTP_BODY_CONTENT_ENCODING_DEFLATE;
	else if(!strncasecmp(value, "compress", sizeof("compress")-1))
		return HTTP_BODY_CONTENT_ENCODING_COMPRESS;
	else if(!strncasecmp(value, "identity", sizeof("identity")-1))
		return HTTP_BODY_CONTENT_ENCODING_IDENTITY;
	else
		return HTTP_BODY_CONTENT_ENCODING_UNKOWN;
}

http_body_content_language_t http_parse_content_language(const char *value)
{
	/*TODO*/
	return HTTP_BODY_CONTENT_LANGUAGE_UNKOWN;
}

void http_parse_content_type(const char *value, http_body_content_maintype_t *main_type, 
					http_body_content_subtype_t *sub_type, 
					http_body_content_charset_t *charset)
{
	if(!value)
		return;
	
	/*TODO: RFC1521*/
	const char *p = value;
	if(!strncasecmp(p, "TEXT/", sizeof("TEXT/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_TEXT;
		p += (sizeof("TEXT/")-1);
	}
	else if(!strncasecmp(p, "APPLICATION/", sizeof("APPLICATION/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_APPLICATION;
		p += (sizeof("APPLICATION/")-1);
	}
	else if(!strncasecmp(p, "MULTIPART/", sizeof("MULTIPART/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_MULTIPART;
		p += (sizeof("MULTIPART/")-1);
	}
	else if(!strncasecmp(p, "IMAGE/", sizeof("IMAGE/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_IMAGE;
		p += (sizeof("IMAGE/")-1);
	}
	else if(!strncasecmp(p, "AUDIO/", sizeof("AUDIO/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_AUDIO;
		p += (sizeof("AUDIO/")-1);
	}
	else if(!strncasecmp(p, "VIDEO/", sizeof("VIDEO/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_VIDEO;
		p += (sizeof("VIDEO/")-1);
	}
	else if(!strncasecmp(p, "DRAWING/", sizeof("DRAWING/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_DRAWING;
		p += (sizeof("DRAWING/")-1);
	}
	else if(!strncasecmp(p, "MODEL/", sizeof("MODEL/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_MODEL;
		p += (sizeof("MODEL/")-1);
	}
	else if(!strncasecmp(p, "MESSAGE/", sizeof("MESSAGE/")-1))
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_MESSAGE;
		p += (sizeof("MESSAGE/")-1);
	}
	else
	{
		*main_type = HTTP_BODY_CONTENT_MAINTYPE_UNKOWN;
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_UNKOWN;
		*charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
		return;
	}


	if(!strncasecmp(p, "HTML", sizeof("HTML")-1))
	{
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_HTML;
		p += (sizeof("HTML")-1);
	}
	else if(!strncasecmp(p, "FORM-DATA", sizeof("FORM-DATA")-1))
	{
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_FORM_DATA;
		p += (sizeof("FORM-DATA")-1);
	}
	else if(!strncasecmp(p, "CSS", sizeof("CSS")-1))
	{
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_CSS;
		p += (sizeof("CSS")-1);
	}
	else if(!strncasecmp(p, "PLAIN", sizeof("PLAIN")-1))
	{
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_PLAIN;
		p += (sizeof("PLAIN")-1);
	}
	else if(!strncasecmp(p, "x-www-form-urlencoded", sizeof("x-www-form-urlencoded")-1))
	{
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_X_WWW_FORM_URLENCODED;
		p += (sizeof("x-www-form-urlencoded")-1);
	}
	else
	{
		*sub_type = HTTP_BODY_CONTENT_SUBTYPE_UNKOWN;
		*charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
		return;
	}

	*charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
	return;
}
