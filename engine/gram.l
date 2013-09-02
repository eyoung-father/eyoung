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
					BEGIN C_COMMENT;
				}
	"%output"	{
					return TOKEN_OUTPUT;
				}
	"%import"	{
					return TOKEN_IMPORT;
				}
	{ident}		{
					return TOKEN_ID;
				}
	{number}	{
					return TOKEN_INT;
				}
	";"			{
					return TOKEN_SEMICOLON;
				}
	":"			{
					return TOKEN_COLON;
				}
	"|"			{
					return TOKEN_PIPE;
				}
	"/"			{
					return TOKEN_SLASH;
				}
	"%%"		{
					if(after_prologue) 
						BEGIN EYOUNG_EPILOGUE; 
					after_prologue++;
					return TOKEN_DPERCENT;
				}
	"\""		{
					BEGIN C_STRING;
				}
	"'"			{
					BEGIN C_CHARACTER;
				}
	"%{"		{
					BEGIN EYOUNG_PROLOGUE;
				}
	"{"			{
					BEGIN EYOUNG_ACTION;
				}
	"("			{
					BEGIN EYOUNG_CONDITION;
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
					BEGIN saved_state; 
					if(!in_code) 
						return TOKEN_STRING;
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
					BEGIN saved_state; 
					if(!in_code) 
						return TOKEN_STRING;
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
					BEGIN saved_state;
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
					BEGIN C_CHARACTER;
				}
	"\""		{
					saved_state = EYOUNG_PROLOGUE;
					BEGIN C_STRING;
				}
	"/*"		{
					saved_state = EYOUNG_PROLOGUE;
					BEGIN C_COMMENT;
				}
	"%}"		{
					BEGIN INITIAL;
					return TOKEN_PROLOGUE_CODE;
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
					BEGIN C_CHARACTER;
				}
	"\""		{
					saved_state = EYOUNG_EPILOGUE;
					BEGIN C_STRING;
				}
	"/*"		{
					saved_state = EYOUNG_EPILOGUE;
					BEGIN C_COMMENT;
				}
	{newline}	{
					yycolumn=1;
				}
	.			{
					/*eat it*/
				}
	<<EOF>>		{
					BEGIN INITIAL;
					return TOKEN_EPILOGUE_CODE;
				}
}

<EYOUNG_ACTION>
{
	/*action state*/
	"'"			{
					saved_state = EYOUNG_ACTION;
					BEGIN C_CHARACTER;
				}
	"\""		{
					saved_state = EYOUNG_ACTION;
					BEGIN C_STRING;
				}
	"/*"		{
					saved_state = EYOUNG_ACTION;
					BEGIN C_COMMENT;
				}
	"{"			{
					brace_level++;
				}
	"}"			{
					brace_level--;
					if(brace_level<=0)
					{
						BEGIN INITIAL;
						return TOKEN_RHS_ACTION;
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
					BEGIN C_CHARACTER;
				}
	"\""		{
					saved_state = EYOUNG_CONDITION;
					BEGIN C_STRING;
				}
	"/*"		{
					saved_state = EYOUNG_CONDITION;
					BEGIN C_COMMENT;
				}
	"("			{
					paren_level++;
				}
	")"			{
					paren_level--;
					if(paren_level<=0)
					{
						BEGIN INITIAL;
						return TOKEN_RHS_CONDITION;
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
