%{
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "pop3.h"
#include "pop3_util.h"
#include "pop3_type.h"
#include "pop3_mem.h"
#include "pop3_server_parser.h"

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
		pop3_debug(debug_pop3_server_lexer,			\
			"line %d get token %s\n",				\
			__LINE__, #val);						\
		if(val==TOKEN_SERVER_STRING)				\
			pop3_debug(debug_pop3_server_lexer,		\
			"get text: %s\n",yytext);				\
		return (val);								\
	}while(0)

#define GOTO(s)										\
	do												\
	{												\
		pop3_debug(debug_pop3_server_lexer, 		\
			"line %d goto state %s(%d)\n",			\
			__LINE__, #s, s);						\
		BEGIN s;									\
	}while(0)
%}
%option header-file="pop3_server_lex.h"
%option outfile="pop3_server_lex.c"
%option prefix="pop3_server_"
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

%x POP3_SERVER_POSITIVE_LINE
%x POP3_SERVER_NEGATIVE_LINE
%x POP3_SERVER_POSITIVE_BODY

SPACE	[ \t]*
NEWLINE	(\r)?\n
LINE	[^\r\n]*
OK		\+OK
ERR		\-ERR
EOB		{SPACE}\.{SPACE}{NEWLINE}
%%
<INITIAL>{
{SPACE}			{
					/*eat blank and tab in the front*/
				}
{NEWLINE}		{
					/*eat newline in the front*/
				}
{OK}{SPACE}		{
					GOTO(POP3_SERVER_POSITIVE_LINE); 
					RETURN(TOKEN_SERVER_OK);
				}
{ERR}{SPACE}	{
					GOTO(POP3_SERVER_NEGATIVE_LINE); 
					RETURN(TOKEN_SERVER_ERROR);
				}
.				{
					yymore();
					GOTO(POP3_SERVER_POSITIVE_BODY);
				}
}

<POP3_SERVER_NEGATIVE_LINE>{
{LINE}			{
					yylval->string.str = yytext;
					yylval->string.str_len = yyleng;
					RETURN(TOKEN_SERVER_STRING);
				}
{NEWLINE}		{
					GOTO(INITIAL);
					RETURN(TOKEN_SERVER_NEWLINE);
				}
.				{
					RETURN(-2);
				}
}

<POP3_SERVER_POSITIVE_LINE>{
{LINE}			{
					yylval->string.str = yytext;
					yylval->string.str_len = yyleng;
					RETURN(TOKEN_SERVER_STRING);
				}
{NEWLINE}		{
					GOTO(INITIAL);
					RETURN(TOKEN_SERVER_NEWLINE);
				}
.				{
					RETURN(-2);
				}
}

<POP3_SERVER_POSITIVE_BODY>{
{EOB}			{
					GOTO(INITIAL); 
					RETURN(TOKEN_SERVER_EOB);
				}
{LINE}			{
					yylval->string.str = yytext;
					yylval->string.str_len = yyleng;
					RETURN(TOKEN_SERVER_STRING);
				}
{NEWLINE}		{
					RETURN(TOKEN_SERVER_NEWLINE);
				}
.				{
					return (-2);
				}
}
%%

void *pop3_server_alloc (yy_size_t sz,yyscan_t yyscanner )
{
	return pop3_malloc(sz);
}

void *pop3_server_realloc (void *ptr,yy_size_t sz,yyscan_t yyscanner )
{
	return pop3_realloc(ptr, sz);
}

void pop3_server_free (void *ptr ,yyscan_t yyscanner )
{
	pop3_free(ptr);
}

struct yy_buffer_state* pop3_server_scan_stream(const char *new_buf, size_t new_buf_len, pop3_data_t *priv)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n, _yybytes_len;
	char *last_buf = priv->response_parser.saved;
	size_t last_buf_len = priv->response_parser.saved_len;
	yyscan_t scanner = (yyscan_t)priv->response_parser.lexier;
    
	_yybytes_len = new_buf_len + last_buf_len;
	n = _yybytes_len + 2;
	buf = (char *)pop3_server_alloc(n, scanner);
	if (!buf)
	{
		pop3_debug(debug_pop3_server_lexer, "out of dynamic memory in pop3_server_scan_stream()\n");
		return NULL;
	}

	if(last_buf)
		memcpy(buf, last_buf, last_buf_len);
	
	if(new_buf)
		memcpy(buf+last_buf_len, new_buf, new_buf_len);

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
	pop3_debug(debug_pop3_server_lexer, "[POP3 SERVER]: %s\n", buf);

	b = pop3_server_scan_buffer(buf, n, scanner);
	if ( ! b )
	{
		pop3_debug(debug_pop3_server_lexer, "bad buffer in pop3_server_scan_bytes()\n");
		pop3_server_free(buf, scanner);
		return NULL;
	}

	b->yy_is_our_buffer = 1;

	if(priv->response_parser.saved)
	{
		pop3_server_free(priv->response_parser.saved, scanner);
		priv->response_parser.saved = NULL;
		priv->response_parser.saved_len = 0;
	}
	return b;
}

static int save_server_stream_context(yyscan_t yyscanner, int from_default)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	pop3_data_t *priv = (pop3_data_t*)pop3_server_get_extra(yyg);
	int len = 0;

	if(priv->response_parser.saved)
	{
		pop3_server_free(priv->response_parser.saved, yyg);
		priv->response_parser.saved = NULL;
		priv->response_parser.saved_len = 0;
	}

	if(!priv || priv->response_parser.last_frag || (!from_default && !priv->response_parser.greedy))
	{
		pop3_debug(debug_pop3_server_lexer, "No need to save stream context, priv: %p, last_frag: %d, from_default: %d\n",
			priv, priv?priv->response_parser.last_frag:0, from_default);
		return 0;
	}

	len = from_default?yyleng-1:yyleng;
	if(!len)
	{
		pop3_debug(debug_pop3_server_lexer, "Exit save stream context for ZERO length yytext\n");
		return 1;
	}

	priv->response_parser.saved = pop3_server_alloc(len, yyg);
	if(!priv->response_parser.saved)
	{
		pop3_debug(debug_pop3_server_lexer, "out of memory while saving context\n");
		return 0;
	}
	memcpy(priv->response_parser.saved, yytext, len);
	priv->response_parser.saved_len = len;
	pop3_debug(debug_pop3_server_lexer, "Save stream context, string: %s, len: %d\n", yytext, len);
	return 1;
}

void pop3_server_lex_fast_init(yyscan_t yyscanner)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	if(!yyg)
		return;
	memset(yyg, 0, sizeof(*yyg));
	yy_init_globals(yyscanner);
}
