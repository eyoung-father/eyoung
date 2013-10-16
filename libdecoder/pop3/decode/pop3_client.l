%{
#include <stdlib.h>
#include <string.h>
#include "pop3.h"
#include "pop3_mem.h"
#include "pop3_type.h"
#include "pop3_util.h"
#include "pop3_client_parser.h"

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
		pop3_debug(debug_pop3_client_lexer,			\
			"line %d get token %s\n",				\
			__LINE__, #val);						\
		if(val==TOKEN_CLIENT_STRING)				\
			pop3_debug(debug_pop3_client_lexer,		\
			"get text: %s\n",yytext);				\
		return (val);								\
	}while(0)

#define GOTO(s)										\
	do												\
	{												\
		pop3_debug(debug_pop3_client_lexer, 		\
			"line %d goto state %s(%d)\n",			\
			__LINE__, #s, s);						\
		BEGIN s;									\
	}while(0)
%}

%option header-file="pop3_client_lex.h"
%option outfile="pop3_client_lex.c"
%option prefix="pop3_client_"
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

%x POP3_CLIENT_USER
%x POP3_CLIENT_PASS
%x POP3_CLIENT_APOP
%x POP3_CLIENT_QUIT
%x POP3_CLIENT_NOOP
%x POP3_CLIENT_STAT
%x POP3_CLIENT_LIST
%x POP3_CLIENT_RETR
%x POP3_CLIENT_DELE
%x POP3_CLIENT_RSET
%x POP3_CLIENT_TOP
%x POP3_CLIENT_UIDL
%x POP3_CLIENT_UNKNOWN

SPACE	[ \t]*
NEWLINE	(\r)?\n
WORD	[^ \r\n\t]+

%%
<INITIAL>{
{SPACE}			{/*eat blank and tab in the front*/}
{NEWLINE}		{/*eat newline*/}
"USER"{SPACE}	{GOTO(POP3_CLIENT_USER); RETURN(TOKEN_CLIENT_USER);}
"PASS"{SPACE}	{GOTO(POP3_CLIENT_PASS); RETURN(TOKEN_CLIENT_PASS);}
"APOP"{SPACE}	{GOTO(POP3_CLIENT_APOP); RETURN(TOKEN_CLIENT_APOP);}
"QUIT"{SPACE}	{GOTO(POP3_CLIENT_QUIT); RETURN(TOKEN_CLIENT_QUIT);}
"NOOP"{SPACE}	{GOTO(POP3_CLIENT_NOOP); RETURN(TOKEN_CLIENT_NOOP);}
"STAT"{SPACE}	{GOTO(POP3_CLIENT_STAT); RETURN(TOKEN_CLIENT_STAT);}
"LIST"{SPACE}	{GOTO(POP3_CLIENT_LIST); RETURN(TOKEN_CLIENT_LIST);}
"RETR"{SPACE}	{GOTO(POP3_CLIENT_RETR); RETURN(TOKEN_CLIENT_RETR);}
"DELE"{SPACE}	{GOTO(POP3_CLIENT_DELE); RETURN(TOKEN_CLIENT_DELE);}
"RSET"{SPACE}	{GOTO(POP3_CLIENT_RSET); RETURN(TOKEN_CLIENT_RSET);}
"TOP"{SPACE}	{GOTO(POP3_CLIENT_TOP); RETURN(TOKEN_CLIENT_TOP);}
"UIDL"{SPACE}	{GOTO(POP3_CLIENT_UIDL); RETURN(TOKEN_CLIENT_UIDL);}
{WORD}{SPACE}	{GOTO(POP3_CLIENT_UNKNOWN); yylval->str=yytext; yylval->str_len=yyleng-1; RETURN(TOKEN_CLIENT_UNKNOWN);}
}

<POP3_CLIENT_USER>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_PASS>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_APOP>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_QUIT>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_NOOP>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_STAT>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_LIST>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_RETR>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_DELE>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_RSET>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_TOP>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_UIDL>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

<POP3_CLIENT_UNKNOWN>{
{WORD}			{yylval->str=yytext; yylval->str_len=yyleng; RETURN(TOKEN_CLIENT_STRING);}
{NEWLINE}		{GOTO(INITIAL); RETURN(TOKEN_CLIENT_NEWLINE);}
}

%%

void *pop3_client_alloc (yy_size_t sz,yyscan_t yyscanner )
{
	return pop3_malloc(sz);
}

void *pop3_client_realloc (void *ptr,yy_size_t sz,yyscan_t yyscanner )
{
	return pop3_realloc(ptr, sz);
}

void pop3_client_free (void *ptr ,yyscan_t yyscanner )
{
	pop3_free(ptr);
}

struct yy_buffer_state* pop3_client_scan_stream(const char *new_buf, size_t new_buf_len, pop3_data_t *priv)
{
	YY_BUFFER_STATE b;
	char *buf;
	yy_size_t n, _yybytes_len;
	char *last_buf = priv->request_parser.saved;
	size_t last_buf_len = priv->request_parser.saved_len;
	yyscan_t scanner = (yyscan_t)priv->request_parser.lexier;
    
	_yybytes_len = new_buf_len + last_buf_len;
	n = _yybytes_len + 2;
	buf = (char *)pop3_client_alloc(n, scanner);
	if (!buf)
	{
		pop3_debug(debug_pop3_client_lexer, "out of dynamic memory in pop3_client_scan_stream()\n");
		return NULL;
	}

	if(last_buf)
		memcpy(buf, last_buf, last_buf_len);
	
	if(new_buf)
		memcpy(buf+last_buf_len, new_buf, new_buf_len);

	buf[_yybytes_len] = buf[_yybytes_len+1] = YY_END_OF_BUFFER_CHAR;
	pop3_debug(debug_pop3_client_lexer, "[POP3 CLIENT]: %s\n", buf);

	b = pop3_client__scan_buffer(buf, n, scanner);
	if ( ! b )
	{
		pop3_debug(debug_pop3_client_lexer, "bad buffer in pop3_client__scan_bytes()\n");
		pop3_client_free(buf, scanner);
		return NULL;
	}

	b->yy_is_our_buffer = 1;

	if(priv->request_parser.saved)
	{
		pop3_client_free(priv->request_parser.saved, scanner);
		priv->request_parser.saved = NULL;
		priv->request_parser.saved_len = 0;
	}
	return b;
}

static int save_client_stream_context(yyscan_t yyscanner, int from_default)
{
	struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
	pop3_data_t *priv = (pop3_data_t*)pop3_client_get_extra(yyg);
	int len = 0;

	if(priv->request_parser.saved)
	{
		pop3_client_free(priv->request_parser.saved, yyg);
		priv->request_parser.saved = NULL;
		priv->request_parser.saved_len = 0;
	}

	if(!priv || priv->request_parser.last_frag || (!from_default && !priv->request_parser.greedy))
	{
		pop3_debug(debug_pop3_client_lexer, "No need to save stream context, priv: %p, last_frag: %d, from_default: %d\n",
			priv, priv?priv->request_parser.last_frag:0, from_default);
		return 0;
	}

	len = from_default?yyleng-1:yyleng;
	if(!len)
	{
		pop3_debug(debug_pop3_client_lexer, "Exit save stream context for ZERO length yytext\n");
		return 1;
	}

	priv->request_parser.saved = pop3_client_alloc(len, yyg);
	if(!priv->request_parser.saved)
	{
		pop3_debug(debug_pop3_client_lexer, "out of memory while saving context\n");
		return 0;
	}
	memcpy(priv->request_parser.saved, yytext, len);
	priv->request_parser.saved_len = len;
	pop3_debug(debug_pop3_client_lexer, "Save stream context, string: %s, len: %d\n", yytext, len);
	return 1;
}
