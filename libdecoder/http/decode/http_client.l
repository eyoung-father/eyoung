%{
#include <stdlib.h>
#include <string.h>
#include "http_client_parser.h"
#include "http.h"

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
		if(val==TOKEN_CLIENT_STRING)				\
			http_debug(debug_http_client_lexer,		\
			"get text: %s\n",yytext);				\
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

%x HTTP_CLIENT_FIRST_METHOD
%x HTTP_CLIENT_FIRST_URI
%x HTTP_CLIENT_FIRST_VERSION
%x HTTP_CLIENT_HEADER_NAME
%x HTTP_CLIENT_HEADER_VALUE
%x HTTP_CLIENT_BODY_NORMAL
%x HTTP_CLIENT_BODY_TRUNK_HEADER
%x HTTP_CLIENT_BODY_TRUNK_BODY
%x HTTP_CLIENT_BODY_TRUNK_END
%x HTTP_CLIENT_BODY_TRUNK_TAILER

%%

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

	b = http_client_scan_buffer(buf, n, scanner);
	if ( ! b )
	{
		http_debug(debug_http_client_lexer, "bad buffer in http_client_scan_bytes()\n");
		http_client_free(buf, scanner);
		return NULL;
	}

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
