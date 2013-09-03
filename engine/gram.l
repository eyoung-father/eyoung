%{
#include <stdio.h>
#include <stdlib.h>

#include "engine_mem.h"
#include "ey_info.h"
#include "gram_parser.h"

#define YY_USER_ACTION										\
	{														\
		yylloc->first_line = yylineno;						\
		yylloc->first_column = yycolumn;					\
		yylloc->last_line = yylineno;						\
		yylloc->last_column = yycolumn+yyleng;				\
		yycolumn += yyleng;									\
	}

#define RETURN(sym)											\
	do														\
	{														\
		engine_lexer_debug("%d-%d:\n", yylineno, yycolumn);	\
		engine_lexer_debug("line %d return %s, value: %s\n",\
			__LINE__, #sym, yytext);						\
		return sym;											\
	}while(0)

#define GOTO(s)												\
	do														\
	{														\
		engine_lexer_debug("%d-%d:\n", yylineno, yycolumn);	\
		engine_lexer_debug("line %d goto state %s(%d)\n",	\
			__LINE__, #s, s);								\
		BEGIN s;											\
	}while(0)

static int brace_level;
static int paren_level;
static int saved_state;
static int in_code;
static int after_prologue;
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

%x EYOUNG_PROLOGUE
%x EYOUNG_EPILOGUE

%x EYOUNG_ACTION
%x EYOUNG_CONDITION

%x C_COMMENT
%x C_STRING
%x C_CHARACTER

space	[ \t\v\f]
newline	\n
letter	[a-zA-Z_]
digit	[0-9]
ident	{letter}({letter}|{digit})*
number	[1-9]{digit}*
%%

<INITIAL>
{
	/*init state*/
	"/*" 		{
					GOTO(C_COMMENT);
				}
	"%output"	{
					RETURN(TOKEN_OUTPUT);
				}
	"%import"	{
					RETURN(TOKEN_IMPORT);
				}
	{ident}		{
					RETURN(TOKEN_ID);
				}
	{number}	{
					RETURN(TOKEN_INT);
				}
	";"			{
					RETURN(TOKEN_SEMICOLON);
				}
	":"			{
					RETURN(TOKEN_COLON);
				}
	"|"			{
					RETURN(TOKEN_PIPE);
				}
	"/"			{
					RETURN(TOKEN_SLASH);
				}
	"%%"		{
					if(after_prologue) 
					{
						GOTO(EYOUNG_EPILOGUE);
						in_code = 1;
					}
					after_prologue++;
					RETURN(TOKEN_DPERCENT);
				}
	"\""		{
					GOTO(C_STRING);
				}
	"'"			{
					GOTO(C_CHARACTER);
				}
	"%{"		{
					GOTO(EYOUNG_PROLOGUE);
					in_code = 1;
				}
	"{"			{
					GOTO(EYOUNG_ACTION);
					in_code = 1;
				}
	"("			{
					GOTO(EYOUNG_CONDITION);
					in_code = 1;
				}
	{space}		{
					/*eat space*/
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*ERROR*/
				}
}

<C_STRING>
{
	/*c language string state*/
	"\\\""		{
					/*eat it*/
				}
	"\""		{
					GOTO(saved_state);
					if(!in_code) 
						RETURN(TOKEN_STRING);
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
}

<C_CHARACTER>
{
	/*c language char state*/
	"\\\'"		{
					/*eat it*/
				}
	"'"			{
					GOTO(saved_state);
					if(!in_code) 
						RETURN(TOKEN_STRING);
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
}

<C_COMMENT>
{
	/*comment*/
	"*/"		{
					GOTO(saved_state);
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
}

<EYOUNG_PROLOGUE>
{
	/*prologue state*/
	"'"			{
					saved_state = EYOUNG_PROLOGUE;
					GOTO(C_CHARACTER);
				}
	"\""		{
					saved_state = EYOUNG_PROLOGUE;
					GOTO(C_STRING);
				}
	"/*"		{
					saved_state = EYOUNG_PROLOGUE;
					GOTO(C_COMMENT);
				}
	"%}"		{
					GOTO(INITIAL);
					saved_state = INITIAL;
					in_code = 0;
					RETURN(TOKEN_PROLOGUE_CODE);
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
}

<EYOUNG_EPILOGUE>
{
	/*epilogue state*/
	"'"			{
					saved_state = EYOUNG_EPILOGUE;
					GOTO(C_CHARACTER);
				}
	"\""		{
					saved_state = EYOUNG_EPILOGUE;
					GOTO(C_STRING);
				}
	"/*"		{
					saved_state = EYOUNG_EPILOGUE;
					GOTO(C_COMMENT);
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
	<<EOF>>		{
					GOTO(INITIAL);
					saved_state = INITIAL;
					in_code = 0;
					RETURN(TOKEN_EPILOGUE_CODE);
				}
}

<EYOUNG_ACTION>
{
	/*action state*/
	"'"			{
					saved_state = EYOUNG_ACTION;
					GOTO(C_CHARACTER);
				}
	"\""		{
					saved_state = EYOUNG_ACTION;
					GOTO(C_STRING);
				}
	"/*"		{
					saved_state = EYOUNG_ACTION;
					GOTO(C_COMMENT);
				}
	"{"			{
					brace_level++;
				}
	"}"			{
					brace_level--;
					if(brace_level<=0)
					{
						GOTO(INITIAL);
						saved_state = INITIAL;
						in_code = 0;
						RETURN(TOKEN_RHS_ACTION);
					}
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
}

<EYOUNG_CONDITION>
{
	/*condition state*/
	"'"			{
					saved_state = EYOUNG_CONDITION;
					GOTO(C_CHARACTER);
				}
	"\""		{
					saved_state = EYOUNG_CONDITION;
					GOTO(C_STRING);
				}
	"/*"		{
					saved_state = EYOUNG_CONDITION;
					GOTO(C_COMMENT);
				}
	"("			{
					paren_level++;
				}
	")"			{
					paren_level--;
					if(paren_level<=0)
					{
						GOTO(INITIAL);
						saved_state = INITIAL;
						in_code = 0;
						RETURN(TOKEN_RHS_CONDITION);
					}
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
}
%%

void *gram_alloc(yy_size_t sz, yyscan_t yyscanner)
{
	return lexer_malloc(sz);
}

void *gram_realloc(void *ptr, yy_size_t sz, yyscan_t yyscanner)
{
	return lexer_realloc(ptr,sz);
}

void gram_free(void *ptr, yyscan_t yyscanner)
{
	lexer_free(ptr);
}
