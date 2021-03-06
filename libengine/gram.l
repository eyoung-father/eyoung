%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "engine_mem.h"
#include "ey_info.h"
#include "ey_loc.h"
#include "ey_signature.h"
#include "libmatch.h"
#include "gram_parser.h"
#include "ey_engine.h"

#define SAVE_LOCATION										\
	do														\
	{														\
		start_location.first_line = yylineno;				\
		start_location.first_column = yycolumn;				\
	}while(0)

#define YY_USER_ACTION										\
	{														\
		yylloc->first_line = start_location.first_line;		\
		yylloc->first_column = start_location.first_column;	\
		yylloc->last_line = yylineno;						\
		yylloc->last_column = yycolumn+yyleng;				\
		yylloc->filename = get_filename(yyscanner);			\
		yycolumn += yyleng;									\
	}

#define RETURN(sym)											\
	do														\
	{														\
		char *ret_str = print_string(yyscanner);			\
		engine_lexer_debug("%d-%d:\n", yylineno, yycolumn);	\
		if(!ret_str || !ret_str[0])							\
			ret_str = yytext;								\
		engine_lexer_debug("line %d return %s, value: %s\n",\
			__LINE__, #sym, ret_str);						\
		reset_string(yyscanner);							\
		SAVE_LOCATION;										\
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
static GRAM_LTYPE start_location = {1,1,1,1,""};

static int add_string(yyscan_t yyscanner, char *str, int str_len);
static void reset_string(yyscan_t yyscanner);
static char *print_string(yyscan_t yyscanner);
static char *copy_string(yyscan_t yyscanner);
static char *get_filename(yyscan_t yyscanner);

#define YY_USER_INIT										\
	do														\
	{														\
		brace_level = 0;									\
		paren_level = 0;									\
		saved_state = 0;									\
		in_code = 0;										\
		after_prologue = 0;									\
		start_location.first_line = 1;						\
		start_location.first_column = 1;					\
		start_location.last_line = 1;						\
		start_location.last_column = 1;						\
		start_location.filename = "";						\
	}while(0)
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
	"/*" 			{
						GOTO(C_COMMENT);
					}
	"%output"		{
						RETURN(TOKEN_OUTPUT);
					}
	"%import"		{
						RETURN(TOKEN_IMPORT);
					}
	"%event"		{
						RETURN(TOKEN_EVENT);
					}
	"%file-init"	{
						RETURN(TOKEN_FILE_INIT);
					}
	"%file-finit"	{
						RETURN(TOKEN_FILE_FINIT);
					}
	"%work-init"	{
						RETURN(TOKEN_WORK_INIT);
					}
	"%work-finit"	{
						RETURN(TOKEN_WORK_FINIT);
					}
	"%event-init"	{
						RETURN(TOKEN_EVENT_INIT);
					}
	"%event-finit"	{
						RETURN(TOKEN_EVENT_FINIT);
					}
	"%event-preprocessor"	{
						RETURN(TOKEN_EVENT_PREPROCESSOR);
					}
	{ident}			{
						ey_engine_t *eng = (ey_engine_t *)gram_get_extra(yyscanner);
						char *ident = (char*)engine_fzalloc(yyleng+1, ey_parser_fslab(eng));
						if(!ident)
						{
							engine_lexer_error("alloc ident name failed\n");
							return -1;
						}
						memcpy(ident, yytext, yyleng);
						ident[yyleng] = '\0';
						yylval->string = ident;
						RETURN(TOKEN_ID);
					}
	{number}		{
						yylval->integer = atoi(yytext);
						RETURN(TOKEN_INT);
					}
	";"				{
						RETURN(TOKEN_SEMICOLON);
					}
	":"				{
						RETURN(TOKEN_COLON);
					}
	"|"				{
						RETURN(TOKEN_PIPE);
					}
	"/"				{
						RETURN(TOKEN_SLASH);
					}
	"%%"			{
						if(after_prologue) 
						{
							GOTO(EYOUNG_EPILOGUE);
							in_code = 1;
						}
						after_prologue++;
						RETURN(TOKEN_DPERCENT);
					}
	"\""			{
						reset_string(yyscanner);
						GOTO(C_STRING);
					}
	"'"				{
						reset_string(yyscanner);
						GOTO(C_CHARACTER);
					}
	"%{"			{
						reset_string(yyscanner);
						GOTO(EYOUNG_PROLOGUE);
						in_code = 1;
					}
	"{"				{
						reset_string(yyscanner);
						add_string(yyscanner, yytext, yyleng);
						GOTO(EYOUNG_ACTION);
						in_code = 1;
						brace_level = 0;
					}
	"("				{
						reset_string(yyscanner);
						add_string(yyscanner, yytext, yyleng);
						GOTO(EYOUNG_CONDITION);
						in_code = 1;
						paren_level = 0;
					}
	{space}			{
						/*eat space*/
					}
	{newline}		{
						yycolumn=1;
					}
	.				{
						engine_lexer_error("bad char %x\n", yytext[0]);
						return -1;
					}
}

<C_STRING>
{
	/*c language string state*/
	"\\\""			{
						/*eat it*/
						add_string(yyscanner, yytext, yyleng);
					}
	"\""			{
						if(in_code)
							add_string(yyscanner, yytext, yyleng);
						GOTO(saved_state);
						if(!in_code) 
						{
							yylval->string = copy_string(yyscanner);
							if(!yylval->string)
								return -1;
							RETURN(TOKEN_STRING);
						}
					}
	{newline}		{
						yycolumn=1;
						add_string(yyscanner, yytext, yyleng);
					}
	.				{
						/*eat it*/
						add_string(yyscanner, yytext, yyleng);
					}
}

<C_CHARACTER>
{
	/*c language char state*/
	"\\\'"			{
						/*eat it*/
						add_string(yyscanner, yytext, yyleng);
					}
	"'"				{
						if(in_code)
							add_string(yyscanner, yytext, yyleng);
						GOTO(saved_state);
						if(!in_code) 
						{
							yylval->string = copy_string(yyscanner);
							if(!yylval->string)
								return -1;
							RETURN(TOKEN_STRING);
						}
					}
	{newline}		{
						yycolumn=1;
						add_string(yyscanner, yytext, yyleng);
					}
	.				{
						/*eat it*/
						add_string(yyscanner, yytext, yyleng);
					}
}

<C_COMMENT>
{
	/*comment*/
	"*/"			{
						if(in_code)
							add_string(yyscanner, yytext, yyleng);
						GOTO(saved_state);
					}
	{newline}		{
						add_string(yyscanner, yytext, yyleng);
						yycolumn=1;
					}
	.				{
						add_string(yyscanner, yytext, yyleng);
						/*eat it*/
					}
}

<EYOUNG_PROLOGUE>
{
	/*prologue state*/
	"'"				{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_PROLOGUE;
						GOTO(C_CHARACTER);
					}
	"\""			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_PROLOGUE;
						GOTO(C_STRING);
					}
	"/*"			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_PROLOGUE;
						GOTO(C_COMMENT);
					}
	"%}"			{
						GOTO(INITIAL);
						saved_state = INITIAL;
						in_code = 0;
						yylval->string = copy_string(yyscanner);
						if(!yylval->string)
							return -1;
						RETURN(TOKEN_PROLOGUE_CODE);
					}
	{newline}		{
						add_string(yyscanner, yytext, yyleng);
						yycolumn=1;
					}
	.				{
						add_string(yyscanner, yytext, yyleng);
						/*eat it*/
					}
}

<EYOUNG_EPILOGUE>
{
	/*epilogue state*/
	"'"				{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_EPILOGUE;
						GOTO(C_CHARACTER);
					}
	"\""			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_EPILOGUE;
						GOTO(C_STRING);
					}
	"/*"			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_EPILOGUE;
						GOTO(C_COMMENT);
					}
	{newline}		{
						add_string(yyscanner, yytext, yyleng);
						yycolumn=1;
					}
	.				{
						add_string(yyscanner, yytext, yyleng);
						/*eat it*/
					}
	<<EOF>>			{
						GOTO(INITIAL);
						saved_state = INITIAL;
						in_code = 0;
						yylval->string = copy_string(yyscanner);
						if(!yylval->string)
							return -1;
						RETURN(TOKEN_EPILOGUE_CODE);
					}
}

<EYOUNG_ACTION>
{
	/*action state*/
	"'"				{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_ACTION;
						GOTO(C_CHARACTER);
					}
	"\""			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_ACTION;
						GOTO(C_STRING);
					}
	"/*"			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_ACTION;
						GOTO(C_COMMENT);
					}
	"{"				{
						add_string(yyscanner, yytext, yyleng);
						brace_level++;
						engine_lexer_debug("add brace level(%d)\n", brace_level);
					}
	"}"				{
						add_string(yyscanner, yytext, yyleng);
						brace_level--;
						engine_lexer_debug("sub brace level(%d)\n", brace_level);
						if(brace_level<0)
						{
							GOTO(INITIAL);
							saved_state = INITIAL;
							in_code = 0;
							yylval->string = copy_string(yyscanner);
							if(!yylval->string)
								return -1;
							RETURN(TOKEN_RHS_ACTION);
						}
					}
	{newline}		{
						add_string(yyscanner, yytext, yyleng);
						yycolumn=1;
					}
	.				{
						/*eat it*/
						add_string(yyscanner, yytext, yyleng);
					}
}

<EYOUNG_CONDITION>
{
	/*condition state*/
	"'"				{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_CONDITION;
						GOTO(C_CHARACTER);
					}
	"\""			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_CONDITION;
						GOTO(C_STRING);
					}
	"/*"			{
						add_string(yyscanner, yytext, yyleng);
						saved_state = EYOUNG_CONDITION;
						GOTO(C_COMMENT);
					}
	"("				{
						add_string(yyscanner, yytext, yyleng);
						paren_level++;
						engine_lexer_debug("add paren level(%d)\n", paren_level);
					}
	")"				{
						add_string(yyscanner, yytext, yyleng);
						paren_level--;
						engine_lexer_debug("sub paren level(%d)\n", paren_level);
						if(paren_level<0)
						{
							GOTO(INITIAL);
							saved_state = INITIAL;
							in_code = 0;
							yylval->string = copy_string(yyscanner);
							if(!yylval->string)
								return -1;
							RETURN(TOKEN_RHS_CONDITION);
						}
					}
	{newline}		{
						add_string(yyscanner, yytext, yyleng);
						yycolumn=1;
					}
	.				{
						add_string(yyscanner, yytext, yyleng);
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

static int add_string(yyscan_t yyscanner, char *str, int str_len)
{
	ey_engine_t *eng = (ey_engine_t *)gram_get_extra(yyscanner);

	if(!eng || !eng->parser || !eng->parser->buffer)
	{
		engine_lexer_error("bad extra parameter\n");
		return -1;
	}

	if(!str || str_len<=0)
	{
		engine_lexer_error("bad string parameter\n");
		return -1;
	}
	
	ey_parser_t *parser = eng->parser;
	if(parser->buffer_len + str_len >= parser->buffer_size)
	{
		char *new_buffer = (char*)engine_fzalloc(parser->buffer_size + str_len + DEFAULT_BUFFER_SIZE, ey_parser_fslab(eng));
		if(!new_buffer)
		{
			engine_lexer_error("alloc new buffer failed\n");
			return -1;
		}

		memcpy(new_buffer, parser->buffer, parser->buffer_len);
		engine_fzfree(ey_parser_fslab(eng), parser->buffer);
		parser->buffer = new_buffer;
		parser->buffer_size = parser->buffer_size + DEFAULT_BUFFER_SIZE + str_len;
	}

	memcpy(parser->buffer + parser->buffer_len, str, str_len);
	parser->buffer_len += str_len;
	parser->buffer[parser->buffer_len] = '\0';
	return 0;
}

static void reset_string(yyscan_t yyscanner)
{
	ey_engine_t *eng = (ey_engine_t *)gram_get_extra(yyscanner);
	if(!eng || !eng->parser || !eng->parser->buffer)
	{
		engine_lexer_error("bad extra parameter\n");
		return;
	}

	eng->parser->buffer_len = 0;
	eng->parser->buffer[0] = '\0';
}

static char *print_string(yyscan_t yyscanner)
{
	ey_engine_t *eng = (ey_engine_t *)gram_get_extra(yyscanner);
	if(!eng || !eng->parser || !eng->parser->buffer)
	{
		engine_lexer_error("bad extra parameter\n");
		return "<NULL>";
	}

	return eng->parser->buffer;
}

static char *copy_string(yyscan_t yyscanner)
{
	ey_engine_t *eng = (ey_engine_t *)gram_get_extra(yyscanner);
	if(!eng || !eng->parser || !eng->parser->buffer)
	{
		engine_lexer_error("bad extra parameter\n");
		return NULL;
	}
	
	char *ret = (char*)engine_fzalloc(eng->parser->buffer_len + 1, ey_parser_fslab(eng));
	if(!ret)
	{
		engine_lexer_error("malloc string failed\n");
		return NULL;
	}
	memcpy(ret, eng->parser->buffer, eng->parser->buffer_len);
	ret[eng->parser->buffer_len] = '\0';
	return ret;
}

static char *get_filename(yyscan_t yyscanner)
{
	ey_engine_t *eng = (ey_engine_t *)gram_get_extra(yyscanner);
	return eng->parser->filename;
}
