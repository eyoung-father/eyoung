%{
#include <stdlib.h>
#include <stdio.h>
#include "http_client_parser.h"
#include "http_server_parser.h"
#include "http_client_lex.h"
#include "http_server_lex.h"

#ifdef YY_REDUCTION_CALLBACK
#undef YY_REDUCTION_CALLBACK
#endif
#define YY_REDUCTION_CALLBACK(data,name,id,val)									\
	do																			\
	{																			\
		http_debug(debug_http_detect, "find attack!\n");						\
	}while(0)

%}
%token TOKEN_CLIENT_CONTINUE

%debug
%verbose
%defines "http_client_parser.h"
%output "http_client_parser.c"
%define api.prefix http_client_
%define api.pure full
%define api.push-pull push
%parse-param {void *priv_data}

%start request_list
%%
request_list:
	{
	}
	;
%%
