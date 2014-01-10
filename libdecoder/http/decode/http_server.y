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

%}

%token TOKEN_SERVER_CONTINUE

%debug
%verbose
%defines "http_server_parser.h"
%output "http_server_parser.c"
%define api.prefix http_server_
%define api.pure full
%define api.push-pull push
%parse-param {void *priv_data}

%start response_list
%%
response_list:
	{
	}
	;
%%
int parse_http_server_stream(http_data_t *priv, const char *buf, size_t buf_len, int last_frag)
{
	http_server_pstate *parser = (http_server_pstate*)priv->response_parser.parser;
	yyscan_t lexier = (yyscan_t)priv->response_parser.lexier;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	HTTP_SERVER_STYPE value;

	yydebug = debug_http_server_parser;
	priv->response_parser.last_frag = last_frag;
	input = http_server_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		http_debug(debug_http_server_parser, "create http server stream buffer failed\n");
		return 1;
	}

	while(1)
	{
		memset(&value, 0, sizeof(value));
		token = http_server_lex(&value, lexier);
		if(token == TOKEN_SERVER_CONTINUE)
			break;
		parser_ret = http_server_push_parse(parser, token, &value, (void*)priv);
		if(parser_ret != YYPUSH_MORE)
			break;
	}
	http_server_delete_buffer(input, lexier);

	if(parser_ret != YYPUSH_MORE && parser_ret != 0)
	{
		http_debug(debug_http_server_parser, "find error while parsing http server stream\n");
		return 2;
	}
	return 0;
}

void http_server_register(http_decoder_t *decoder)
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
			http_debug(debug_http_server_parser, "server event id of %s is %d\n", name, yytid[YYNTOKENS + index]);
		else
			http_debug(debug_http_server_parser, "failed to register server event %s\n", name);
	}
}
#undef priv_decoder
