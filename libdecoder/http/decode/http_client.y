%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "http.h"
#include "http_private.h"
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
		if(http_element_detect(data,name,id,val,								\
			cluster_buffer,cluster_buffer_len)<0)								\
		{																		\
			http_debug(debug_http_detect, "find attack!\n");					\
			return -1;															\
		}																		\
	}while(0)

#define priv_decoder															\
	((http_decoder_t*)(((http_data_t*)priv_data)->decoder))

int http_cmd_pair_id;
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
int parse_http_client_stream(http_data_t *priv, const char *buf, size_t buf_len, int last_frag)
{
	http_client_pstate *parser = (http_client_pstate*)priv->request_parser.parser;
	yyscan_t lexier = (yyscan_t)priv->request_parser.lexier;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	HTTP_CLIENT_STYPE value;

	yydebug = debug_http_client_parser;
	priv->request_parser.last_frag = last_frag;
	input = http_client_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		http_debug(debug_http_client_parser, "create http client stream buffer failed\n");
		return 1;
	}

	while(1)
	{
		memset(&value, 0, sizeof(value));
		token = http_client_lex(&value, lexier);
		if(token == TOKEN_CLIENT_CONTINUE)
			break;
		parser_ret = http_client_push_parse(parser, token, &value, (void*)priv);
		if(parser_ret != YYPUSH_MORE)
			break;
	}
	http_client_delete_buffer(input, lexier);

	if(parser_ret != YYPUSH_MORE && parser_ret != 0)
	{
		http_debug(debug_http_client_parser, "find error while parsing http client stream\n");
		return 2;
	}
	return 0;
}

void http_client_register(http_decoder_t *decoder)
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
			http_debug(debug_http_server_parser, "client event id of %s is %d\n", name, yytid[YYNTOKENS + index]);
		else
			http_debug(debug_http_server_parser, "failed to register client event %s\n", name);
	}

	http_cmd_pair_id = ey_engine_find_event(engine, "cmd_pair");
}
#undef priv_decoder
