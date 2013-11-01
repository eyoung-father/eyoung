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
#define YY_REDUCTION_CALLBACK(data,name,id,val)									\
	do																			\
	{																			\
		pop3_debug(debug_pop3_detect, "detect server %s[%d]\n", name, id);		\
	}while(0)

#define priv_decoder															\
	((pop3_decoder_t*)(((pop3_data_t*)priv_data)->decoder))
%}

%token TOKEN_SERVER_EOB
%token TOKEN_SERVER_OK
%token TOKEN_SERVER_ERROR
%token TOKEN_SERVER_STRING

%token TOKEN_SERVER_NEWLINE
%token TOKEN_SERVER_CONTINUE

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
	pop3_string_t string;
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
	pop3_free_response_content(priv_decoder, &$$);
}positive_response_lines positive_response_message

%destructor
{
	pop3_free_response(priv_decoder, $$);
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
		/*do nothing*/
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

positive_response: TOKEN_SERVER_OK positive_response_line
	{
		pop3_data_t *data = (pop3_data_t*)priv_data;
		pop3_response_t *res = pop3_alloc_response(priv_decoder, POP3_RESPONSE_OK, $2.str, $2.str_len, NULL);
		if(!res)
		{
			pop3_debug(debug_pop3_server_parser, "failed to alloc positive response\n");
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&data->response_list, res, next);

		/*DO make pop3 command pair*/
		if(pop3_add_command(priv_decoder, data))
		{
			pop3_debug(debug_pop3_server_parser, "add pop3 command failed\n");
			YYABORT;
		}
		$<response>$ = res;
	}
	TOKEN_SERVER_NEWLINE positive_response_message
	{
		STAILQ_CONCAT(&$<response>3->content, &$5);
		$$ = $<response>3;
	}
	;

positive_response_line: TOKEN_SERVER_STRING 
	{
		char *data = NULL;
		int data_len = $1.str_len;

		if(data_len)
		{
			data = (char*)pop3_malloc(data_len + 1);
			if(!data)
			{
				pop3_debug(debug_pop3_server_parser, "failed to alloc positive response line data\n");
				YYABORT;
			}
			memcpy(data, $1.str, data_len);
			data[data_len] = '\0';
		}
		$$.str = data;
		$$.str_len = data_len;
	}
	| 
	{
		$$.str = NULL;
		$$.str_len = 0;
	}
	;

positive_response_message: 
	{
		STAILQ_INIT(&$$);
	}
	| TOKEN_SERVER_EOB
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
				pop3_debug(debug_pop3_server_parser, "failed to alloc positive response line data\n");
				YYABORT;
			}
			memcpy(data, $1.str, data_len);
			data[data_len] = '\0';
		}
		
		pop3_line_t *line = pop3_alloc_response_line(priv_decoder, data, data_len);
		if(!line)
		{
			pop3_debug(debug_pop3_server_parser, "failed to alloc positive response line\n");
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
				pop3_debug(debug_pop3_server_parser, "failed to alloc positive response line data\n");
				YYABORT;
			}
			memcpy(data, $2.str, data_len);
			data[data_len] = '\0';
		}
		
		pop3_line_t *line = pop3_alloc_response_line(priv_decoder, data, data_len);
		if(!line)
		{
			pop3_debug(debug_pop3_server_parser, "failed to alloc positive response line\n");
			pop3_free(data);
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&$1, line, next);
		$$ = $1;
	}
	;

negative_response: TOKEN_SERVER_ERROR negative_response_line
	{
		pop3_data_t *data = (pop3_data_t*)priv_data;
		pop3_response_t *res = pop3_alloc_response(priv_decoder, POP3_RESPONSE_ERROR, $2.str, $2.str_len, NULL);
		if(!res)
		{
			pop3_debug(debug_pop3_server_parser, "failed to alloc negative response\n");
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&data->response_list, res, next);

		/*DO make pop3 command pair*/
		if(pop3_add_command(priv_decoder, data))
		{
			pop3_debug(debug_pop3_server_parser, "add pop3 command failed\n");
			YYABORT;
		}
		$<response>$ = res;
	}
	TOKEN_SERVER_NEWLINE
	{
		$$ = $<response>3;
	}
	;

negative_response_line: TOKEN_SERVER_STRING 
	{
		char *data = NULL;
		int data_len = $1.str_len;

		if(data_len)
		{
			data = (char*)pop3_malloc(data_len + 1);
			if(!data)
			{
				pop3_debug(debug_pop3_server_parser, "failed to alloc negative response line data\n");
				YYABORT;
			}
			memcpy(data, $1.str, data_len);
			data[data_len] = '\0';
		}
		$$.str = data;
		$$.str_len = data_len;
	}
	| 
	{
		$$.str = NULL;
		$$.str_len = 0;
	}
	;
%%
int parse_pop3_server_stream(pop3_data_t *priv, const char *buf, size_t buf_len, int last_frag)
{
	pop3_server_pstate *parser = (pop3_server_pstate*)priv->response_parser.parser;
	yyscan_t lexier = (yyscan_t)priv->response_parser.lexier;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	POP3_SERVER_STYPE value;

	yydebug = debug_pop3_server_parser;
	priv->response_parser.last_frag = last_frag;
	input = pop3_server_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		pop3_debug(debug_pop3_server_parser, "create pop3 server stream buffer failed\n");
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
		pop3_debug(debug_pop3_server_parser, "find error while parsing pop3 server stream\n");
		return 2;
	}
	return 0;
}

void pop3_server_register(pop3_decoder_t *decoder)
{
	assert(decoder!=NULL);
	assert(decoder->engine!=NULL);

	engine_t engine = decoder->engine;
	int index = 0;
	for(index=0; index<YYNNTS; index++)
	{
		const char *name = yytname[YYNTOKENS + index];
		if(!name || name[0]=='$' || name[0]=='@')
			continue;
		yytid[YYNTOKENS + index] = ey_engine_find_event(engine, name);
		if(yytid[YYNTOKENS + index] >= 0)
			pop3_debug(debug_pop3_server_parser, "server event id of %s is %d\n", name, yytid[YYNTOKENS + index]);
		else
			pop3_debug(debug_pop3_server_parser, "failed to register server event %s\n", name);
	}
}
#undef priv_decoder
