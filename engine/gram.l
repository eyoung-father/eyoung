%{
#include <stdio.h>
#include <stdlib.h>

#include "gram_parser.h"

#define YY_USER_ACTION							\
	{											\
		yylloc->first_line = yylineno;			\
		yylloc->first_column = yycolumn;		\
		yylloc->last_line = yylineno;			\
		yylloc->last_column = yycolumn+yyleng;	\
		yycolumn += yyleng;						\
	}
%}

%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option never-interactive
%option header-file="gram_lexer.h"
%option outfile="gram_lexer.c"
%option prefix="gram_"
%option bison-bridge
%option bison-locations
%option reentrant
%option ansi-definitions
%option ansi-prototypes
%option noyyalloc
%option noyyfree
%option noyyrealloc
%option yylineno
%%
%%
