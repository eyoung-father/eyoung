%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "pop3.h"
#include "pop3_detect.h"
#include "pop3_util.h"
#include "pop3_type.h"
#include "pop3_mem.h"
#include "pop3_client_parser.h"
#include "pop3_server_parser.h"
#include "pop3_client_lex.h"
#include "pop3_server_lex.h"
#include "pop3_private.h"

#ifdef YY_REDUCTION_CALLBACK
#undef YY_REDUCTION_CALLBACK
#endif
#define YY_REDUCTION_CALLBACK(name,val)											\
	do																			\
	{																			\
		if(pop3_do_rule_detect((pop3_data_t*)priv_data, name, (void*)val) < 0)	\
		{																		\
			pop3_attack(debug_pop3_ips, "find attack by signature\n");			\
			YYABORT;															\
		}																		\
	}while(0)
%}

%token TOKEN_SERVER_EOB			"."
%token TOKEN_SERVER_OK			"+OK"
%token TOKEN_SERVER_ERROR		"-ERR"
%token TOKEN_SERVER_STRING		"-string-"

%token TOKEN_SERVER_NEWLINE		"-newline-"
%token TOKEN_SERVER_CONTINUE	"-continue-"

%debug
%verbose
%defines "pop3_server_parser.h"
%output "pop3_server_parser.c"
%define api.prefix pop3_server_
%define api.pure full
%define api.push-pull push
%parse-param {void *priv_data}

%union
{
	struct
	{
		char *str;
		int str_len;
	}string;
	pop3_res_content_t content;
	pop3_response_t *response;
}

%type <string>		positive_response_line
					negative_response_line
					TOKEN_SERVER_STRING
%type <content>		positive_response_lines
					positive_response_message
%type <response>	positive_response
					negative_response
					response

%destructor
{
	if($$.str)
		pop3_free($$.str);
}positive_response_line negative_response_line

%destructor
{
	pop3_free_response_content(&$$);
}positive_response_lines positive_response_message

%destructor
{
	pop3_free_response($$);
}positive_response negative_response

%start response_list
%%
response_list:
	{
		pop3_data_t *data = (pop3_data_t*)priv_data;
		STAILQ_INIT(&data->response_list);
	}
	| response_list response
	{
		pop3_data_t *data = (pop3_data_t*)priv_data;
		STAILQ_INSERT_TAIL(&data->response_list, $2, next);
		
		/*DO make pop3 command pair*/
		if(pop3_add_command(data))
		{
			pop3_debug(debug_pop3_server, "add pop3 command failed\n");
			YYABORT;
		}
	}
	;

response: positive_response
	{
		$$ = $1;
	}
	| negative_response
	{
		$$ = $1;
	}
	;

positive_response: TOKEN_SERVER_OK positive_response_line positive_response_message
	{
		pop3_response_t *res = pop3_alloc_response(POP3_RESPONSE_OK, $2.str, $2.str_len, &$3);
		if(!res)
		{
			pop3_debug(debug_pop3_server, "failed to alloc positive response\n");
			YYABORT;
		}
		$$ = res;
	}
	| TOKEN_SERVER_OK positive_response_line
	{
		pop3_response_t *res = pop3_alloc_response(POP3_RESPONSE_OK, $2.str, $2.str_len, NULL);
		if(!res)
		{
			pop3_debug(debug_pop3_server, "failed to alloc positive response\n");
			YYABORT;
		}
		$$ = res;
	}
	;

positive_response_line: TOKEN_SERVER_STRING TOKEN_SERVER_NEWLINE
	{
		char *data = NULL;
		int data_len = $1.str_len;

		if(data_len)
		{
			data = (char*)pop3_malloc(data_len + 1);
			if(!data)
			{
				pop3_debug(debug_pop3_server, "failed to alloc positive response line data\n");
				YYABORT;
			}
			memcpy(data, $1.str, data_len);
			data[data_len] = '\0';
		}
		$$.str = data;
		$$.str_len = data_len;
	}
	| TOKEN_SERVER_NEWLINE
	{
		$$.str = NULL;
		$$.str_len = 0;
	}
	;

positive_response_message: 
	TOKEN_SERVER_EOB
	{
		STAILQ_INIT(&$$);
	}
	| positive_response_lines TOKEN_SERVER_EOB
	{
		$$ = $1;
	}
	;

positive_response_lines: TOKEN_SERVER_STRING TOKEN_SERVER_NEWLINE
	{
		char *data = NULL;
		int data_len = $1.str_len;

		if(data_len)
		{
			data = (char*)pop3_malloc(data_len + 1);
			if(!data)
			{
				pop3_debug(debug_pop3_server, "failed to alloc positive response line data\n");
				YYABORT;
			}
			memcpy(data, $1.str, data_len);
			data[data_len] = '\0';
		}
		
		pop3_line_t *line = pop3_alloc_response_line(data, data_len);
		if(!line)
		{
			pop3_debug(debug_pop3_server, "failed to alloc positive response line\n");
			pop3_free(data);
			YYABORT;
		}
		STAILQ_INIT(&$$);
		STAILQ_INSERT_TAIL(&$$, line, next);
	}
	| positive_response_lines TOKEN_SERVER_STRING TOKEN_SERVER_NEWLINE
	{
		char *data = NULL;
		int data_len = $2.str_len;

		if(data_len)
		{
			data = (char*)pop3_malloc(data_len + 1);
			if(!data)
			{
				pop3_debug(debug_pop3_server, "failed to alloc positive response line data\n");
				YYABORT;
			}
			memcpy(data, $2.str, data_len);
			data[data_len] = '\0';
		}
		
		pop3_line_t *line = pop3_alloc_response_line(data, data_len);
		if(!line)
		{
			pop3_debug(debug_pop3_server, "failed to alloc positive response line\n");
			pop3_free(data);
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&$1, line, next);
		$$ = $1;
	}
	;

negative_response: TOKEN_SERVER_ERROR negative_response_line negative_response_message
	{
		pop3_response_t *res = pop3_alloc_response(POP3_RESPONSE_ERROR, $2.str, $2.str_len, NULL);
		if(!res)
		{
			pop3_debug(debug_pop3_server, "failed to alloc negative response\n");
			YYABORT;
		}
		$$ = res;
	}
	;

negative_response_line: TOKEN_SERVER_STRING TOKEN_SERVER_NEWLINE
	{
		char *data = NULL;
		int data_len = $1.str_len;

		if(data_len)
		{
			data = (char*)pop3_malloc(data_len + 1);
			if(!data)
			{
				pop3_debug(debug_pop3_server, "failed to alloc negative response line data\n");
				YYABORT;
			}
			memcpy(data, $1.str, data_len);
			data[data_len] = '\0';
		}
		$$.str = data;
		$$.str_len = data_len;
	}
	| TOKEN_SERVER_NEWLINE
	{
		$$.str = NULL;
		$$.str_len = 0;
	}
	;

negative_response_message:
	| negative_response_lines
	;

negative_response_lines: TOKEN_SERVER_STRING TOKEN_SERVER_NEWLINE
	| negative_response_lines TOKEN_SERVER_STRING TOKEN_SERVER_NEWLINE
	;
%%
int parse_pop3_server_stream(pop3_data_t *priv, const char *buf, size_t buf_len, int last_frag)
{
	pop3_server_pstate *parser = (pop3_server_pstate*)priv->response_parser.parser;
	yyscan_t lexier = (yyscan_t)priv->response_parser.lexier;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	POP3_SERVER_STYPE value;

	yydebug = debug_pop3_server;
	priv->response_parser.last_frag = last_frag;
	input = pop3_server_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		pop3_debug(debug_pop3_server, "create pop3 server stream buffer failed\n");
		return 1;
	}

	while(1)
	{
		memset(&value, 0, sizeof(value));
		token = pop3_server_lex(&value, lexier);
		if(token == TOKEN_SERVER_CONTINUE)
			break;
		parser_ret = pop3_server_push_parse(parser, token, &value, (void*)priv);
		if(parser_ret != YYPUSH_MORE)
			break;
	}
	pop3_server__delete_buffer(input, lexier);

	if(parser_ret != YYPUSH_MORE && parser_ret != 0)
	{
		pop3_debug(debug_pop3_server, "find error while parsing pop3 server stream\n");
		return 2;
	}
	return 0;
}
