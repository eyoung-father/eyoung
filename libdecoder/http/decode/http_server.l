%{
#include <stdlib.h>
#include <stdio.h>
#include "http_server_parser.h"

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

%%

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
