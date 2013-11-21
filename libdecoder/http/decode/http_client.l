%{
#include <stdlib.h>
#include <string.h>
#include "http_client_parser.h"

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
