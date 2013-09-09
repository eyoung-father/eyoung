%{
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "engine_mem.h"
#include "ey_memory.h"
#include "ey_loc.h"
#include "ey_info.h"
#include "ey_signature.h"

int gram_error(GRAM_LTYPE *loc, void *eng, const char *format, ...);

#undef yydebug
#define yydebug debug_engine_parser
%}

%token TOKEN_STRING			"string"
%token TOKEN_INT			"integer"
%token TOKEN_ID				"event"

%token TOKEN_SEMICOLON		";"
%token TOKEN_COLON			":"
%token TOKEN_PIPE			"|"
%token TOKEN_DPERCENT		"%%"
%token TOKEN_SLASH			"/"
%token TOKEN_PROLOGUE_CODE	"%{...%}"
%token TOKEN_RHS_CONDITION	"(...)"
%token TOKEN_RHS_ACTION		"{...}"
%token TOKEN_EPILOGUE_CODE	"epilogue"

%token TOKEN_OUTPUT			"%output"
%token TOKEN_IMPORT			"%import"
%token TOKEN_EVENT			"%event"

%union
{
	char *string;
	int integer;
	ey_rhs_item_t *rhs_item;
	ey_rhs_item_action_t *action;
	ey_rhs_item_condition_t *condition;
	ey_rhs_signature_t *rhs_signature;
	ey_rhs_signature_list_t rhs_list;
	ey_signature_t *signature;
	ey_signature_list_t signature_list;
	ey_code_t *code;
	ey_code_list_t code_list;
	ey_signature_file_t *file;
}

%type	<integer>			TOKEN_INT
%type	<string>			TOKEN_ID
%type	<string>			TOKEN_STRING
%type	<string>			TOKEN_RHS_ACTION
%type	<string>			TOKEN_PROLOGUE_CODE
%type	<string>			TOKEN_RHS_CONDITION
%type	<string>			TOKEN_EPILOGUE_CODE

%type	<string>			rhs_name
%type	<string>			rhs_cluster_opt
%destructor
{
	if($$)
		lexer_free($$);
}rhs_name rhs_cluster_opt

%type	<action>			rhs_action_opt
%destructor
{
	ey_free_rhs_item_action($$);
}rhs_action_opt

%type	<condition>			rhs_condition_opt
%destructor
{
	ey_free_rhs_item_condition($$);
}rhs_condition_opt

%type	<rhs_item>			signature_rhs
%destructor
{
	ey_free_rhs_item($$);
}signature_rhs

%type	<rhs_signature>		signature_rhs_list
%destructor
{
	ey_free_rhs_signature($$);
}signature_rhs_list

%type	<rhs_list>			signature_pipe_list
%destructor
{
	ey_rhs_signature_t *rhs=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(rhs, &$$, link, tmp)
		ey_free_rhs_signature(rhs);
}signature_pipe_list

%type	<integer>			signature_lhs

%type	<signature>			signature
%destructor
{
	ey_free_signature($$);
}signature

%type	<signature_list>	signatures
%type	<signature_list>	signature_opt
%destructor
{
	ey_signature_t *sig=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(sig, &$$, link, tmp)
		ey_free_signature(sig);
}signatures signature_opt

%type	<code>				prologue
%type	<code>				epilogue_opt
%destructor
{
	ey_free_code($$);
}prologue

%type	<code_list>			prologue_list
%type	<code_list>			prologue_opt
%destructor
{
	ey_code_t *code=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(code, &$$, link, tmp)
		ey_free_code(code);
}prologue_list prologue_opt

%type	<file>				eyoung_file
%destructor
{
	ey_free_signature_file($$);
}eyoung_file

%debug
%verbose
%locations
%defines "gram_parser.h"
%output "gram_parser.c"
%define api.prefix gram_
%define api.pure full
%define api.push-pull push
%parse-param {void *eng}

%start eyoung_file
%%
empty:
	;

eyoung_file:
	prologue_opt TOKEN_DPERCENT signature_opt TOKEN_DPERCENT epilogue_opt
	{
		ey_signature_file_t *ret = ey_alloc_signature_file(NULL, &$1, &$3, $5);
		if(!ret)
		{
			engine_parser_error("alloc file failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	;

prologue_opt:
	empty
	{
		TAILQ_INIT(&$$);
	}
	| prologue_list
	{
		$$ = $1;
	}
	;

prologue_list:
	prologue
	{
		ey_code_t *code = $1;
		TAILQ_INIT(&$$);
		if(code)
			TAILQ_INSERT_TAIL(&$$, code, link);
	}
	| prologue_list prologue
	{
		if($2)
			TAILQ_INSERT_TAIL(&$1, $2, link);
		$$ = $1;
	}
	;

prologue:
	TOKEN_PROLOGUE_CODE
	{
		ey_code_t *ret = ey_alloc_code(&@1, $1);
		if(!ret)
		{
			engine_parser_error("alloc prologue code failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	| TOKEN_OUTPUT TOKEN_STRING
	{
		/*TODO: set output filename*/
		$$ = NULL;
	}
	| TOKEN_IMPORT TOKEN_STRING
	{
		/*TODO: do import*/
		$$ = NULL;
	}
	| TOKEN_EVENT TOKEN_STRING TOKEN_STRING
	{
		/*TODO: event declaration*/
		$$ = NULL;
	}
	;

signature_opt:
	empty
	{
		TAILQ_INIT(&$$);
	}
	| signatures
	{
		$$ = $1;
	}
	;

signatures:
	signature
	{
		ey_signature_t *sig = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, sig, link);
	}
	| signatures signature
	{
		TAILQ_INSERT_TAIL(&$1, $2, link);
		$$ = $1;
	}
	;

signature:
	signature_lhs TOKEN_COLON signature_pipe_list TOKEN_SEMICOLON
	{
		ey_signature_t *ret = ey_alloc_signature($1, &@1, &$3);
		if(!ret)
		{
			engine_parser_error("alloc signature failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	;

signature_lhs:
	TOKEN_INT
	{
		$$ = $1;
	}
	;

signature_pipe_list:
	signature_rhs_list
	{
		ey_rhs_signature_t *rhs = $1;
		TAILQ_INIT(&$$);
		TAILQ_INSERT_TAIL(&$$, rhs, link);
	}
	| signature_pipe_list TOKEN_PIPE signature_rhs_list
	{
		TAILQ_INSERT_TAIL(&$1, $3, link);
		$$ = $1;
	}
	;

signature_rhs_list:
	signature_rhs
	{
		ey_rhs_signature_t *ret = ey_alloc_rhs_signature(&@1, NULL);
		if(!ret)
		{
			engine_parser_error("alloc rhs signature failed");
			YYABORT;
		}
		TAILQ_INSERT_TAIL(&ret->rhs_item_list, $1, link);
		$$ = ret;
	}
	| signature_rhs_list signature_rhs
	{
		TAILQ_INSERT_TAIL(&$1->rhs_item_list, $2, link);
		$$ = $1;
	}
	;

signature_rhs:
	rhs_name rhs_condition_opt rhs_cluster_opt rhs_action_opt
	{
		ey_rhs_item_t *ret = ey_alloc_rhs_item(&@1, $1, $3, $2, $4);
		if(!ret)
		{
			engine_parser_error("alloc rhs item failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	;

rhs_name:
	TOKEN_ID
	{
		/*TODO: do event name check*/
		$$ = $1;
	}
	;

rhs_condition_opt:
	empty
	{
		$$ = NULL;
	}
	| TOKEN_RHS_CONDITION
	{
		ey_rhs_item_condition_t *ret = ey_alloc_rhs_item_condition(&@1, $1, NULL);
		if(!ret)
		{
			engine_parser_error("alloc condition failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	;

rhs_cluster_opt:
	empty
	{
		$$ = NULL;
	}
	| TOKEN_SLASH TOKEN_STRING
	{
		$$ = $2;
	}
	;

rhs_action_opt:
	empty
	{
		$$ = NULL;
	}
	| TOKEN_RHS_ACTION
	{
		ey_rhs_item_action_t *ret = ey_alloc_rhs_item_action(&@1, $1, NULL);
		if(!ret)
		{
			engine_parser_error("alloc action failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	;

epilogue_opt:
	empty
	{
		$$ = NULL;
	}
	| TOKEN_EPILOGUE_CODE
	{
		ey_code_t *ret = ey_alloc_code(&@1, $1);
		if(!ret)
		{
			engine_parser_error("alloc epilogue code failed\n");
			YYABORT;
		}
		$$ = ret;
	}
	;
%%

int gram_error(GRAM_LTYPE *loc, void *eng, const char *format, ...)
{
	char error_buf[4096];
	va_list ap;

	va_start(ap, format);
	vsnprintf(error_buf, sizeof(error_buf)-1, format, ap);
	error_buf[sizeof(error_buf)-1] = '\0';
	va_end(ap);

	return engine_parser_error("error(line %d): %s\n", loc->first_line, error_buf);
}
