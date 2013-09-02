%{
#include <stdio.h>
#include <stdlib.h>

#include "ey_memory.h"
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
	;

prologue_opt:
	empty
	| prologue_list
	;

prologue_list:
	prologue
	| prologue_list prologue
	;

prologue:
	TOKEN_PROLOGUE_CODE
	;

signature_opt:
	empty
	| signatures
	;

signatures:
	signature
	| signatures signature
	;

signature:
	signature_lhs TOKEN_COLON signature_pipe_list TOKEN_SEMICOLON
	;

signature_lhs:
	TOKEN_INT
	;

signature_pipe_list:
	signature_rhs_list
	| signature_pipe_list TOKEN_PIPE signature_rhs_list
	;

signature_rhs_list:
	signature_rhs
	| signature_rhs_list signature_rhs
	;

signature_rhs:
	rhs_name rhs_condition_opt rhs_cluster_opt rhs_action_opt
	;

rhs_name:
	TOKEN_ID
	;

rhs_condition_opt:
	empty
	| rhs_condition
	;

rhs_condition:
	TOKEN_RHS_CONDITION
	;

rhs_cluster_opt:
	empty
	| TOKEN_SLASH rhs_cluster
	;

rhs_cluster:
	TOKEN_STRING
	;

rhs_action_opt:
	empty
	| rhs_action
	;

rhs_action:
	TOKEN_RHS_ACTION
	;

epilogue_opt:
	empty
	| epilogue
	;

epilogue:
	TOKEN_EPILOGUE_CODE
	;
%%
