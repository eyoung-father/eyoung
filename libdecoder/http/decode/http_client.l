%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "http.h"
#include "http_client_parser.h"
#include "http_private.h"

extern char *strcasestr (const char *__haystack, const char *__needle);
static int save_client_stream_context(yyscan_t yyscanner, int from_default);

#define YY_USER_ACTION								\
	if(yyg->yy_hold_char==YY_END_OF_BUFFER_CHAR &&	\
			save_client_stream_context(yyg,0))		\
		return TOKEN_CLIENT_CONTINUE;

#ifdef YY_RESTORE_YY_MORE_OFFSET
	#undef YY_RESTORE_YY_MORE_OFFSET
	#define YY_RESTORE_YY_MORE_OFFSET				\
	{												\
		if(save_client_stream_context(yyg,1))		\
			return TOKEN_CLIENT_CONTINUE;			\
	}
#endif

#define YY_USER_INIT								\
	{												\
		yyout = stderr;								\
	}

#define RETURN(val)									\
	do												\
	{												\
		http_debug(debug_http_client_lexer,			\
			"line %d get token %s\n",				\
			__LINE__, #val);						\
		return (val);								\
	}while(0)

#define GOTO(s)										\
	do												\
	{												\
		http_debug(debug_http_client_lexer, 		\
			"line %d goto state %s(%d)\n",			\
			__LINE__, #s, s);						\
		BEGIN s;									\
	}while(0)

%}

%option header-file="http_client_lex.h"
%option outfile="http_client_lex.c"
%option prefix="http_client_"
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

%x HTTP_CLIENT_FIRST_URI
%x HTTP_CLIENT_FIRST_VERSION
%x HTTP_CLIENT_HEADER_NAME
%x HTTP_CLIENT_HEADER_VALUE
%x HTTP_CLIENT_BODY_NORMAL
%x HTTP_CLIENT_BODY_TRUNK_HEADER
%x HTTP_CLIENT_BODY_TRUNK_BODY
%x HTTP_CLIENT_BODY_TRUNK_END
%x HTTP_CLIENT_BODY_TRUNK_TAILER
%x HTTP_CLIENT_CONTENT_LENGTH_HEADER_VALUE
%x HTTP_CLIENT_TRANSFER_ENCODING_HEADER_VALUE

SPACE	[ \t]*
SPACE1	[ \t]+
NEWLINE	(\r)?\n
WORD	[^ \r\n\t]+

%%

<INITIAL>{
"GET"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_GET);}
"POST"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_POST);}
"HEAD"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_HEAD);}
"OPTIONS"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_OPTIONS);}
"PUT"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_PUT);}
"DELETE"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_DELETE);}
"TRACE"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_TRACE);}
"CONNECT"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_CONNECT);}
"PATCH"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_PATCH);}
"LINK"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_LINK);}
"UNLINK"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_UNLINK);}
"PROPFIND"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_PROPFIND);}
"PROPPATCH"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_PROPPATCH);}
"MKCOL"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_MKCOL);}
"COPY"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_COPY);}
"MOVE"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_MOVE);}
"LOCK"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_LOCK);}
"UNLOCK"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_UNLOCK);}
"CHECKOUT"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_CHECKOUT);}
"REPORT"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_REPORT);}
"VERSION-CONTROL"{SPACE1}					{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_VERSION_CONTROL);}
"CHECKIN"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_CHECKIN);}
"UNCHECKOUT"{SPACE1}						{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_UNCHECKOUT);}
"MKWORKSPACE"{SPACE1}						{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_MKWORKSPACE);}
"UPDATE"{SPACE1}							{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_UPDATE);}
"LABEL"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_LABEL);}
"MERGE"{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_MERGE);}
"BASELINE-CONTROL"{SPACE1}					{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_BASELINE_CONTROL);}
"MKACTIVITY"{SPACE1}						{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_MKACTIVITY);}
"ORDERPATCH"{SPACE1}						{GOTO(HTTP_CLIENT_FIRST_URI); RETURN(TOKEN_CLIENT_FIRST_METHOD_ORDERPATCH);}
.											{RETURN(-1);}
}

<HTTP_CLIENT_FIRST_URI>{
{WORD}{SPACE1}								{GOTO(HTTP_CLIENT_FIRST_VERSION); RETURN(TOKEN_CLINET_FIRST_URI);}
.											{RETURN(-1);}
}

<HTTP_CLIENT_FIRST_VERSION>{
"HTTP/0.9"{NEWLINE}							{GOTO(HTTP_CLIENT_HEADER_NAME); RETURN(TOKEN_CLIENT_FIRST_VERSION_09);}
"HTTP/1.0"{NEWLINE}							{GOTO(HTTP_CLIENT_HEADER_NAME); RETURN(TOKEN_CLIENT_FIRST_VERSION_10);}
"HTTP/1.1"{NEWLINE}							{GOTO(HTTP_CLIENT_HEADER_NAME); RETURN(TOKEN_CLIENT_FIRST_VERSION_11);}
.											{RETURN(-1);}
}

<HTTP_CLIENT_HEADER_NAME>{
{SPACE}"HOST"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_HOST);}
{SPACE}"CACHE-CONTROL"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CACHE_CONTROL);}
{SPACE}"CONNECTION"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONNECTION);}
{SPACE}"DATE"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_DATE);}
{SPACE}"PRAGMA"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_PRAGMA);}
{SPACE}"TRAILER"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_TRAILER);}
{SPACE}"TRANSFER-ENCODING"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_TRANSFER_ENCODING_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_TRANSFER_ENCODING);}
{SPACE}"UPGRADE"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_UPGRADE);}
{SPACE}"VIA"{SPACE}:{SPACE}					{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_VIA);}
{SPACE}"WARNING"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_WARNING);}
{SPACE}"MIME-VERSION"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_MIME_VERSION);}
{SPACE}"ALLOW"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_ALLOW);}
{SPACE}"CONTENT-ENCODING"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_ENCODING);}
{SPACE}"CONTENT-LANGUAGE"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_LANGUAGE);}
{SPACE}"CONTENT-LENGTH"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_CONTENT_LENGTH_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_LENGTH);}
{SPACE}"CONTENT-LOCATION"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_LOCATION);}
{SPACE}"CONTENT-MD5"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_MD5);}
{SPACE}"CONTENT-RANGE"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_RANGE);}
{SPACE}"CONTENT-TYPE"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_TYPE);}
{SPACE}"ETAG"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_ETAG);}
{SPACE}"EXPIRES"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_EXPIRES);}
{SPACE}"LAST-MODIFIED"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_LAST_MODIFIED);}
{SPACE}"CONTENT-BASE"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_BASE);}
{SPACE}"CONTENT-VERSION"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_CONTENT_VERSION);}
{SPACE}"DERIVED-FROM"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_DERIVED_FROM);}
{SPACE}"LINK"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_LINK);}
{SPACE}"KEEP-ALIVE"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_KEEP_ALIVE);}
{SPACE}"URI"{SPACE}:{SPACE}					{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_URI);}
{SPACE}"ACCEPT-CHARSET"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_ACCEPT_CHARSET);}
{SPACE}"ACCEPT-ENCODING"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_ACCEPT_ENCODING);}
{SPACE}"ACCEPT-LANGUAGE"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_ACCEPT_LANGUAGE);}
{SPACE}"ACCEPT"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_ACCEPT);}
{SPACE}"AUTHORIZATION"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_AUTHORIZATION);}
{SPACE}"EXCEPT"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_EXCEPT);}
{SPACE}"FROM"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_FROM);}
{SPACE}"IF-MATCH"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_IF_MATCH);}
{SPACE}"IF-MODIFIED-SINCE"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_IF_MODIFIED_SINCE);}
{SPACE}"IF-NONE-MATCH"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_IF_NONE_MATCH);}
{SPACE}"IF-RANGE"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_IF_RANGE);}
{SPACE}"IF-UNMODIFIED-SINCE"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_IF_UNMODIFIED_SINCE);}
{SPACE}"MAX-FORWARDS"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_MAX_FORWARDS);}
{SPACE}"RANGE"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_RANGE);}
{SPACE}"REFERER"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_REFERER);}
{SPACE}"TE"{SPACE}:{SPACE}					{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_TE);}
{SPACE}"USER-AGENT"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_USER_AGENT);}
{SPACE}"COOKIE2"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_COOKIE2);}
{SPACE}"COOKIE"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_COOKIE);}
{SPACE}"UA-PIXELS"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_UA_PIXELS);}
{SPACE}"UA-COLOR"{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_UA_COLOR);}
{SPACE}"UA-OS"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_UA_OS);}
{SPACE}"UA-CPU"{SPACE}:{SPACE}				{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_UA_CPU);}
{SPACE}"X-FLASH-VERSION"{SPACE}:{SPACE}		{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_X_FLASH_VERSION);}
{SPACE}"PROXY-AUTHORIZATION"{SPACE}:{SPACE}	{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_PROXY_AUTHORIZATION);}
{SPACE}[a-zA-Z\-_]+{SPACE}:{SPACE}			{GOTO(HTTP_CLIENT_HEADER_VALUE); RETURN(TOKEN_CLIENT_HEADER_UNKOWN);}
{NEWLINE}									{
												if(http_client_lexer_is_chunk_body(yyg))
													GOTO(HTTP_CLIENT_BODY_TRUNK_HEADER);
												else 
													GOTO(HTTP_CLIENT_BODY_NORMAL);
												RETURN(TOKEN_CLIENT_HEADER_TERM);
											}
.											{RETURN(-1);}
}

<HTTP_CLIENT_CONTENT_LENGTH_HEADER_VALUE>{
[^\r\n]*{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_client_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_client_lexer, "prepare content-length header value failed\n");
													RETURN(-1);
												}

												int error=0;
												size_t length = http_parse_integer(yytext, 10, &error);
												if(error)
												{
													http_debug(debug_http_client_lexer, "parse content-length value failed\n");
													RETURN(-1);
												}
												http_client_lexer_set_content_length(yyg, length);
												GOTO(HTTP_CLIENT_HEADER_NAME);
												RETURN(TOKEN_CLIENT_HEADER_VALUE);
											}
.											{RETURN(-1);}
}

<HTTP_CLIENT_TRANSFER_ENCODING_HEADER_VALUE>{
[^\r\n]*{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_client_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_client_lexer, "prepare header value failed\n");
													RETURN(-1);
												}
												if(strcasestr(yytext, "chunked"))
													http_client_lexer_set_chunked_body(yyg);
												GOTO(HTTP_CLIENT_HEADER_NAME);
												RETURN(TOKEN_CLIENT_HEADER_VALUE);
											}
.											{RETURN(-1);}
}

<HTTP_CLIENT_HEADER_VALUE>{
[^\r\n]*{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_client_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_client_lexer, "prepare header value failed\n");
													RETURN(-1);
												}
												GOTO(HTTP_CLIENT_HEADER_NAME);
												RETURN(TOKEN_CLIENT_HEADER_VALUE);
											}
.											{RETURN(-1);}
}

<HTTP_CLIENT_BODY_NORMAL>{
.											{RETURN(-1);}
}

<HTTP_CLIENT_BODY_TRUNK_HEADER>{
[^\r\n]+{NEWLINE}							{
												http_data_t *priv_data = (http_data_t *)yyget_extra(yyg);
												http_decoder_t *decoder = (http_decoder_t*)priv_data->decoder;
												if(http_client_prepare_string(decoder, yytext, yyleng, &yylval->string, 1))
												{
													http_debug(debug_http_client_lexer, "prepare chunk header value failed\n");
													RETURN(-1);
												}

												int error=0;
												size_t length = http_parse_integer(yytext, 16, &error);
												if(error)
												{
													http_debug(debug_http_client_lexer, "parse chunk length value failed\n");
													RETURN(-1);
												}
												http_client_lexer_set_chunk_length(yyg, length);
												if(length)
													GOTO(HTTP_CLIENT_BODY_TRUNK_BODY);
												else
													GOTO(HTTP_CLIENT_BODY_TRUNK_TAILER);
												RETURN(TOKEN_CLIENT_BODY_CHUNK_HEADER);
											}
.											{RETURN(-1);}
}

<HTTP_CLIENT_BODY_TRUNK_BODY>{
{NEWLINE}									{GOTO(HTTP_CLIENT_BODY_TRUNK_HEADER);}
.											{RETURN(-1);}
}

<HTTP_CLIENT_BODY_TRUNK_TAILER>{
[^\r\n]+{NEWLINE}							{RETURN(TOKEN_CLIENT_BODY_CHUNK_TAILER);}
{NEWLINE}									{GOTO(INITIAL); http_client_lexer_init_body_info(yyg); RETURN(TOKEN_CLIENT_BODY_END);}
.											{RETURN(-1);}
}

%%

void *http_client_alloc (yy_size_t sz,yyscan_t yyscanner )
{
	return http_malloc(sz);
}

void *http_client_realloc (void *ptr,yy_size_t sz,yyscan_t yyscanner )
{
	return http_realloc(ptr, sz);
}

void http_client_free (void *ptr ,yyscan_t yyscanner )
{
	http_free(ptr);
}

struct yy_buffer_state* http_client_scan_stream(const char *new_buf, size_t new_buf_len, http_data_t *priv)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n, _yybytes_len;
	char *last_buf = priv->request_parser.saved;
	size_t last_buf_len = priv->request_parser.saved_len;
	yyscan_t scanner = (yyscan_t)priv->request_parser.lexier;
    
	_yybytes_len = new_buf_len + last_buf_len;
	n = _yybytes_len + 2;
	buf = (char *)http_client_alloc(n, scanner);
	if (!buf)
	{
		http_debug(debug_http_client_lexer, "out of dynamic memory in http_client_scan_stream()\n");
		return NULL;
	}

	if(last_buf)
		memcpy(buf, last_buf, last_buf_len);
	
	if(new_buf)
		memcpy(buf+last_buf_len, new_buf, new_buf_len);

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
	http_debug(debug_http_client_lexer, "[HTTP CLIENT]: %s\n", buf);

	//b = http_client_scan_buffer(buf, n, scanner);
	b = (YY_BUFFER_STATE)http_client_alloc(sizeof(struct yy_buffer_state), scanner);
	if ( ! b )
	{
		http_debug(debug_http_client_lexer, "failed to alloc client buffer state\n");
		http_client_free(buf, scanner);
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
	http_client_switch_to_buffer(b, scanner);

	b->yy_is_our_buffer = 1;

	if(priv->request_parser.saved)
	{
		http_client_free(priv->request_parser.saved, scanner);
		priv->request_parser.saved = NULL;
		priv->request_parser.saved_len = 0;
	}
	return b;
}

static int save_client_stream_context(yyscan_t yyscanner, int from_default)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	http_data_t *priv = (http_data_t*)http_client_get_extra(yyg);
	int len = 0;

	if(priv->request_parser.saved)
	{
		http_client_free(priv->request_parser.saved, yyg);
		priv->request_parser.saved = NULL;
		priv->request_parser.saved_len = 0;
	}

	if(!priv || priv->request_parser.last_frag || (!from_default && !priv->request_parser.greedy))
	{
		http_debug(debug_http_client_lexer, "No need to save stream context, priv: %p, last_frag: %d, from_default: %d\n",
			priv, priv?priv->request_parser.last_frag:0, from_default);
		return 0;
	}

	len = from_default?yyleng-1:yyleng;
	if(!len)
	{
		http_debug(debug_http_client_lexer, "Exit save stream context for ZERO length yytext\n");
		return 1;
	}

	priv->request_parser.saved = http_client_alloc(len, yyg);
	if(!priv->request_parser.saved)
	{
		http_debug(debug_http_client_lexer, "out of memory while saving context\n");
		return 0;
	}
	memcpy(priv->request_parser.saved, yytext, len);
	priv->request_parser.saved_len = len;
	http_debug(debug_http_client_lexer, "Save stream context, string: %s, len: %d\n", yytext, len);
	return 1;
}

int http_client_lex_body_mode(yyscan_t scanner)
{
	struct yyguts_t *yyg = (struct yyguts_t*)scanner;
	int state = YY_START;
	http_data_t *priv_data = (http_data_t*)http_client_get_extra(scanner);
	http_parser_t *parser = &priv_data->request_parser;
	if(state == HTTP_CLIENT_BODY_NORMAL)
	{
		assert(!parser->chunked);
		return 1;
	}

	if(state == HTTP_CLIENT_BODY_TRUNK_BODY)
	{
		assert(parser->chunked);
		return (parser->length != 0);
	}
	return 0;
}

int http_client_body_lex(HTTP_CLIENT_STYPE *val, yyscan_t scanner)
{
	struct yyguts_t *yyg = (struct yyguts_t*)scanner;
	YY_BUFFER_STATE buffer = YY_CURRENT_BUFFER;
	http_data_t *priv_data = (http_data_t*)http_client_get_extra(scanner);
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
		http_debug(debug_http_client_lexer, "read all data, return TOKEN_CLIENT_BODY_END\n");
		GOTO(INITIAL);
		return TOKEN_CLIENT_BODY_END;
	}
	
	http_debug(debug_http_client_lexer, "will read %d bytes in this buffer state\n", read);
	buffer->yy_buf_pos = yyg->yy_c_buf_p;
	*buffer->yy_buf_pos = yyg->yy_hold_char;
	http_client_prepare_string(decoder, buffer->yy_buf_pos, read, string, 0);
	buffer->yy_buf_pos += read;
	parser->length -= read;
	yy_load_buffer_state(scanner);
	return TOKEN_CLIENT_BODY_PART;
}
