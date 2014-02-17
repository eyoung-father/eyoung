%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "http.h"
#include "http_server_parser.h"
#include "http_private.h"

extern char *strcasestr (const char *__haystack, const char *__needle);
static int save_server_stream_context(yyscan_t yyscanner, int from_default);

#define YY_USER_ACTION								\
	if(yyg->yy_hold_char==YY_END_OF_BUFFER_CHAR &&	\
			save_server_stream_context(yyg,0))		\
		return TOKEN_SERVER_CONTINUE;

#ifdef YY_RESTORE_YY_MORE_OFFSET
	#undef YY_RESTORE_YY_MORE_OFFSET
	#define YY_RESTORE_YY_MORE_OFFSET				\
	{												\
		if(save_server_stream_context(yyg,1))		\
			return TOKEN_SERVER_CONTINUE;			\
	}
#endif

#define YY_USER_INIT								\
	{												\
		yyout = stderr;								\
	}

#define RETURN(val)									\
	do												\
	{												\
		http_debug(debug_http_server_lexer,			\
			"line %d get token %s\n",				\
			__LINE__, #val);						\
		return (val);								\
	}while(0)

#define GOTO(s)										\
	do												\
	{												\
		http_debug(debug_http_server_lexer, 		\
			"line %d goto state %s(%d)\n",			\
			__LINE__, #s, s);						\
		BEGIN s;									\
	}while(0)

%}

%option header-file="http_server_lex.h"
%option outfile="http_server_lex.c"
%option prefix="http_server_"
%option bison-bridge
%option noyywrap
%option reentrant
%option case-insensitive
%option ansi-definitions
%option noinput
%option nounput
%option noyyalloc
%option noyyfree
%option noyyrealloc

%x HTTP_SERVER_FIRST_CODE
%x HTTP_SERVER_FIRST_MESSAGE
%x HTTP_SERVER_HEADER_NAME
%x HTTP_SERVER_HEADER_VALUE
%x HTTP_SERVER_BODY_NORMAL
%x HTTP_SERVER_BODY_TRUNK_HEADER
%x HTTP_SERVER_BODY_TRUNK_BODY
%x HTTP_SERVER_BODY_TRUNK_END
%x HTTP_SERVER_BODY_TRUNK_TAILER
%x HTTP_SERVER_CONTENT_LENGTH_HEADER_VALUE
%x HTTP_SERVER_TRANSFER_ENCODING_HEADER_VALUE

SPACE	[ \t]*
SPACE1	[ \t]+
NEWLINE	(\r)?\n
WORD	[^ \r\n\t]+
NUMBER	[1-9][0-9][0-9]

%%

<INITIAL>{
"HTTP/0.9"{SPACE1}							{GOTO(HTTP_SERVER_FIRST_CODE); RETURN(TOKEN_SERVER_FIRST_VERSION_09);}
"HTTP/1.0"{SPACE1}							{GOTO(HTTP_SERVER_FIRST_CODE); RETURN(TOKEN_SERVER_FIRST_VERSION_10);}
"HTTP/1.1"{SPACE1}							{GOTO(HTTP_SERVER_FIRST_CODE); RETURN(TOKEN_SERVER_FIRST_VERSION_11);}
.											{RETURN(-1);}
}

<HTTP_SERVER_FIRST_CODE>{
{NUMBER}{SPACE1}							{GOTO(HTTP_SERVER_FIRST_MESSAGE); RETURN(TOKEN_SERVER_FIRST_CODE);}
.											{RETURN(-1);}
}

<HTTP_SERVER_FIRST_MESSAGE>{
[^\r\n]*{NEWLINE}							{GOTO(HTTP_SERVER_HEADER_NAME); RETURN(TOKEN_SERVER_FIRST_VALUE);}
.											{RETURN(-1);}
}

<HTTP_SERVER_HEADER_NAME>{
{SPACE}"CACHE-CONTROL"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CACHE_CONTROL);}
{SPACE}"CONNECTION"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONNECTION);}
{SPACE}"DATE"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_DATE);}
{SPACE}"PRAGMA"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_PRAGMA);}
{SPACE}"TRAILER"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_TRAILER);}
{SPACE}"TRANSFER-ENCODING"{SPACE}:{SPACE}	{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_TRANSFER_ENCODING);}
{SPACE}"UPGRADE"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_UPGRADE);}
{SPACE}"VIA"{SPACE}:{SPACE}					{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_VIA);}
{SPACE}"WARNING"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_WARNING);}
{SPACE}"MIME-VERSION"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_MIME_VERSION);}
{SPACE}"ALLOW"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_ALLOW);}
{SPACE}"CONTENT-ENCODING"{SPACE}:{SPACE}	{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_ENCODING);}
{SPACE}"CONTENT-LANGUAGE"{SPACE}:{SPACE}	{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_LANGUAGE);}
{SPACE}"CONTENT-LENGTH"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_LENGTH);}
{SPACE}"CONTENT-LOCATION"{SPACE}:{SPACE}	{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_LOCATION);}
{SPACE}"CONTENT-MD5"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_MD5);}
{SPACE}"CONTENT-RANGE"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_RANGE);}
{SPACE}"CONTENT-TYPE"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_TYPE);}
{SPACE}"ETAG"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_ETAG);}
{SPACE}"EXPIRES"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_EXPIRES);}
{SPACE}"LAST-MODIFIED"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_LAST_MODIFIED);}
{SPACE}"CONTENT-BASE"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_BASE);}
{SPACE}"CONTENT-VERSION"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_CONTENT_VERSION);}
{SPACE}"DERIVED-FROM"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_DERIVED_FROM);}
{SPACE}"LINK"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_LINK);}
{SPACE}"KEEP-ALIVE"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_KEEP_ALIVE);}
{SPACE}"URI"{SPACE}:{SPACE}					{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_URI);}
{SPACE}"ACCEPT-RANGES"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_ACCEPT_RANGES);}
{SPACE}"AGE"{SPACE}:{SPACE}					{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_AGE);}
{SPACE}"LOCATION"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_LOCATION);}
{SPACE}"RETRY-AFTER"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_RETRY_AFTER);}
{SPACE}"SERVER"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_SERVER);}
{SPACE}"VARY"{SPACE}:{SPACE}				{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_VARY);}
{SPACE}"WWW-AUTHENTICATE"{SPACE}:{SPACE}	{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_WWW_AUTHENTICATE);}
{SPACE}"SET-COOKIE2"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_SET_COOKIE2);}
{SPACE}"SET-COOKIE"{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_SET_COOKIE);}
{SPACE}"X-POWERED-BY"{SPACE}:{SPACE}		{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_X_POWERED_BY);}
{SPACE}"PROXY-AUTHENTICATE"{SPACE}:{SPACE}	{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_PROXY_AUTHENTICATE);}
{SPACE}[a-zA-Z\-_]+{SPACE}:{SPACE}			{GOTO(HTTP_SERVER_HEADER_VALUE); RETURN(TOKEN_SERVER_HEADER_UNKOWN);}
{NEWLINE}									{
												if(http_server_lexer_is_chunk_body(yyg))
													GOTO(HTTP_SERVER_BODY_TRUNK_HEADER);
												else
													GOTO(HTTP_SERVER_BODY_NORMAL); 
												RETURN(TOKEN_SERVER_HEADER_TERM);
											}
.											{RETURN(-1);}
}

<HTTP_SERVER_CONTENT_LENGTH_HEADER_VALUE>{
[^\r\n]*{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_server_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_server_lexer, "prepare content-length header value failed\n");
													RETURN(-1);
												}

												int error=0;
												size_t length = http_parse_integer(yytext, 10, &error);
												if(error)
												{
													http_debug(debug_http_server_lexer, "parse content-length value failed\n");
													RETURN(-1);
												}
												http_server_lexer_set_content_length(yyg, length);
												GOTO(HTTP_SERVER_HEADER_NAME);
												RETURN(TOKEN_SERVER_HEADER_VALUE);
											}
.											{RETURN(-1);}
}


<HTTP_SERVER_TRANSFER_ENCODING_HEADER_VALUE>{
[^\r\n]*{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_server_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_server_lexer, "prepare header value failed\n");
													RETURN(-1);
												}
												if(strcasestr(yytext, "chunked"))
													http_server_lexer_set_chunked_body(yyg);
												GOTO(HTTP_SERVER_HEADER_NAME);
												RETURN(TOKEN_SERVER_HEADER_VALUE);
												GOTO(HTTP_SERVER_HEADER_NAME);
												RETURN(TOKEN_SERVER_HEADER_VALUE);
											}
.											{RETURN(-1);}
}

<HTTP_SERVER_HEADER_VALUE>{
[^\r\n]*{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_server_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_server_lexer, "prepare header value failed\n");
													RETURN(-1);
												}
												GOTO(HTTP_SERVER_HEADER_NAME);
												RETURN(TOKEN_SERVER_HEADER_VALUE);
											}
.											{RETURN(-1);}
}

<HTTP_SERVER_BODY_NORMAL>{
.											{RETURN(-1);}
}

<HTTP_SERVER_BODY_TRUNK_HEADER>{
[^\r\n]+{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_server_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_server_lexer, "prepare chunk header value failed\n");
													RETURN(-1);
												}

												int error=0;
												size_t length = http_parse_integer(yytext, 16, &error);
												if(error)
												{
													http_debug(debug_http_server_lexer, "parse chunk length value failed\n");
													RETURN(-1);
												}
												http_server_lexer_set_chunk_length(yyg, length);
												if(length)
													GOTO(HTTP_SERVER_BODY_TRUNK_BODY);
												else
													GOTO(HTTP_SERVER_BODY_TRUNK_TAILER);
												RETURN(TOKEN_SERVER_BODY_CHUNK_HEADER);
											}
.											{RETURN(-1);}
}

<HTTP_SERVER_BODY_TRUNK_BODY>{
{NEWLINE}									{GOTO(HTTP_SERVER_BODY_TRUNK_HEADER);}
.											{RETURN(-1);}
}

<HTTP_SERVER_BODY_TRUNK_TAILER>{
[^\r\n]+{NEWLINE}							{RETURN(TOKEN_SERVER_BODY_CHUNK_TAILER);}
{NEWLINE}									{GOTO(INITIAL); http_server_lexer_init_body_info(yyg); RETURN(TOKEN_SERVER_BODY_END);}
.											{RETURN(-1);}
}

%%

void *http_server_alloc (yy_size_t sz,yyscan_t yyscanner )
{
	return http_malloc(sz);
}

void *http_server_realloc (void *ptr,yy_size_t sz,yyscan_t yyscanner )
{
	return http_realloc(ptr, sz);
}

void http_server_free (void *ptr ,yyscan_t yyscanner )
{
	http_free(ptr);
}

struct yy_buffer_state* http_server_scan_stream(const char *new_buf, size_t new_buf_len, http_data_t *priv)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n, _yybytes_len;
	char *last_buf = priv->response_parser.saved;
	size_t last_buf_len = priv->response_parser.saved_len;
	yyscan_t scanner = (yyscan_t)priv->response_parser.lexier;
    
	_yybytes_len = new_buf_len + last_buf_len;
	n = _yybytes_len + 2;
	buf = (char *)http_server_alloc(n, scanner);
	if (!buf)
	{
		http_debug(debug_http_server_lexer, "out of dynamic memory in http_server_scan_stream()\n");
		return NULL;
	}

	if(last_buf)
		memcpy(buf, last_buf, last_buf_len);
	
	if(new_buf)
		memcpy(buf+last_buf_len, new_buf, new_buf_len);

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
	http_debug(debug_http_server_lexer, "[HTTP SERVER]: %s\n", buf);

	//b = http_server_scan_buffer(buf, n, scanner);
	b = (YY_BUFFER_STATE)http_server_alloc(sizeof(struct yy_buffer_state), scanner);
	if ( ! b )
	{
		http_debug(debug_http_server_lexer, "failed to alloc server buffer state\n");
		http_server_free(buf, scanner);
		return NULL;
	}

	b->yy_buf_size = n - 2;	/* "- 2" to take care of EOB's */
	b->yy_buf_pos = b->yy_ch_buf = buf;
	b->yy_is_our_buffer = 0;
	b->yy_input_file = 0;
	b->yy_n_chars = b->yy_buf_size;
	b->yy_is_interactive = 0;
	b->yy_at_bol = 1;
	b->yy_fill_buffer = 0;
	b->yy_buffer_status = YY_BUFFER_NEW;
	http_server_switch_to_buffer(b, scanner);

	b->yy_is_our_buffer = 1;

	if(priv->response_parser.saved)
	{
		http_server_free(priv->response_parser.saved, scanner);
		priv->response_parser.saved = NULL;
		priv->response_parser.saved_len = 0;
	}
	return b;
}

static int save_server_stream_context(yyscan_t yyscanner, int from_default)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	http_data_t *priv = (http_data_t*)http_server_get_extra(yyg);
	int len = 0;

	if(priv->response_parser.saved)
	{
		http_server_free(priv->response_parser.saved, yyg);
		priv->response_parser.saved = NULL;
		priv->response_parser.saved_len = 0;
	}

	if(!priv || priv->response_parser.last_frag || (!from_default && !priv->response_parser.greedy))
	{
		http_debug(debug_http_server_lexer, "No need to save stream context, priv: %p, last_frag: %d, from_default: %d\n",
			priv, priv?priv->response_parser.last_frag:0, from_default);
		return 0;
	}

	len = from_default?yyleng-1:yyleng;
	if(!len)
	{
		http_debug(debug_http_server_lexer, "Exit save stream context for ZERO length yytext\n");
		return 1;
	}

	priv->response_parser.saved = http_server_alloc(len, yyg);
	if(!priv->response_parser.saved)
	{
		http_debug(debug_http_server_lexer, "out of memory while saving context\n");
		return 0;
	}
	memcpy(priv->response_parser.saved, yytext, len);
	priv->response_parser.saved_len = len;
	http_debug(debug_http_server_lexer, "Save stream context, string: %s, len: %d\n", yytext, len);
	return 1;
}

void http_server_lex_fast_init(yyscan_t yyscanner)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if(!yyg)
		return;
	memset(yyg, 0, sizeof(*yyg));
	yy_init_globals(yyscanner);
}

int http_server_lex_body_mode(yyscan_t scanner)
{
	struct yyguts_t *yyg = (struct yyguts_t*)scanner;
	int state = YY_START;
	http_data_t *priv_data = (http_data_t*)http_server_get_extra(scanner);
	http_parser_t *parser = &priv_data->request_parser;
	if(state == HTTP_SERVER_BODY_NORMAL)
	{
		assert(!parser->chunked);
		return 1;
	}

	if(state == HTTP_SERVER_BODY_TRUNK_BODY)
	{
		assert(parser->chunked);
		return (parser->length != 0);
	}
	return 0;
}

int http_server_body_lex(HTTP_SERVER_STYPE *val, yyscan_t scanner)
{
	struct yyguts_t *yyg = (struct yyguts_t*)scanner;
	YY_BUFFER_STATE buffer = YY_CURRENT_BUFFER;
	http_data_t *priv_data = (http_data_t*)http_server_get_extra(scanner);
	http_parser_t *parser = &priv_data->request_parser;
	size_t left = buffer->yy_buf_size - (yyg->yy_c_buf_p - buffer->yy_ch_buf);
	size_t need = parser->length;
	size_t read = need;
	http_string_t *string = &val->string;
	http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;

	if(need>left)
		read = left;

	if(read == 0)
	{
		assert(parser->chunked == 0);
		http_debug(debug_http_server_lexer, "read all data, return TOKEN_SERVER_BODY_END\n");
		GOTO(INITIAL);
		return TOKEN_SERVER_BODY_END;
	}
	
	http_debug(debug_http_server_lexer, "will read %d bytes in this buffer state\n", read);
	buffer->yy_buf_pos = yyg->yy_c_buf_p;
	*buffer->yy_buf_pos = yyg->yy_hold_char;
	http_server_prepare_string(decoder, buffer->yy_buf_pos, read, string, 0);
	buffer->yy_buf_pos += read;
	parser->length -= read;
	yy_load_buffer_state(scanner);
	return TOKEN_SERVER_BODY_PART;
}
