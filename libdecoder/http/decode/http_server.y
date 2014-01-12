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

%token TOKEN_SERVER_HEADER_CACHE_CONTROL
%token TOKEN_SERVER_HEADER_CONNECTION
%token TOKEN_SERVER_HEADER_DATE
%token TOKEN_SERVER_HEADER_PRAGMA
%token TOKEN_SERVER_HEADER_TRAILER
%token TOKEN_SERVER_HEADER_TRANSFER_ENCODING
%token TOKEN_SERVER_HEADER_UPGRADE
%token TOKEN_SERVER_HEADER_VIA
%token TOKEN_SERVER_HEADER_WARNING
%token TOKEN_SERVER_HEADER_MIME_VERSION
%token TOKEN_SERVER_HEADER_ALLOW
%token TOKEN_SERVER_HEADER_CONTENT_ENCODING
%token TOKEN_SERVER_HEADER_CONTENT_LANGUAGE
%token TOKEN_SERVER_HEADER_CONTENT_LENGTH
%token TOKEN_SERVER_HEADER_CONTENT_LOCATION
%token TOKEN_SERVER_HEADER_CONTENT_MD5
%token TOKEN_SERVER_HEADER_CONTENT_RANGE
%token TOKEN_SERVER_HEADER_CONTENT_TYPE
%token TOKEN_SERVER_HEADER_ETAG
%token TOKEN_SERVER_HEADER_EXPIRES
%token TOKEN_SERVER_HEADER_LAST_MODIFIED
%token TOKEN_SERVER_HEADER_CONTENT_BASE
%token TOKEN_SERVER_HEADER_CONTENT_VERSION
%token TOKEN_SERVER_HEADER_DERIVED_FROM
%token TOKEN_SERVER_HEADER_LINK
%token TOKEN_SERVER_HEADER_KEEP_ALIVE
%token TOKEN_SERVER_HEADER_URI
%token TOKEN_SERVER_HEADER_ACCEPT_RANGES
%token TOKEN_SERVER_HEADER_AGE
%token TOKEN_SERVER_HEADER_LOCATION
%token TOKEN_SERVER_HEADER_RETRY_AFTER
%token TOKEN_SERVER_HEADER_SERVER
%token TOKEN_SERVER_HEADER_VARY
%token TOKEN_SERVER_HEADER_WWW_AUTHENTICATE
%token TOKEN_SERVER_HEADER_SET_COOKIE2
%token TOKEN_SERVER_HEADER_SET_COOKIE
%token TOKEN_SERVER_HEADER_X_POWERED_BY
%token TOKEN_SERVER_HEADER_PROXY_AUTHENTICATE

%token TOKEN_SERVER_FIRST_VERSION_09
%token TOKEN_SERVER_FIRST_VERSION_10
%token TOKEN_SERVER_FIRST_VERSION_11

%token TOKEN_SERVER_FIRST_CODE
%token TOKEN_SERVER_FIRST_VALUE

%token TOKEN_SERVER_HEADER_COLON
%token TOKEN_SERVER_HEADER_VALUE
%token TOKEN_SERVER_HEADER_TERM

%token TOKEN_SERVER_BODY_PART

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
	empty
	{
	}
	| response_list response
	{
	}
	;

empty:
	{
	}
	;

response:
	response_line response_headers response_body
	{
	}
	;

response_line:
	response_line_version response_line_code response_line_message
	{
	}
	;

response_line_version:
	TOKEN_SERVER_FIRST_VERSION_09
	{
	}
	| TOKEN_SERVER_FIRST_VERSION_10
	{
	}
	| TOKEN_SERVER_FIRST_VERSION_11
	{
	}
	;

response_line_code:
	TOKEN_SERVER_FIRST_CODE
	{
	}
	;

response_line_message:
	TOKEN_SERVER_FIRST_VALUE
	{
	}
	;

response_headers:
	response_header_list TOKEN_SERVER_HEADER_TERM
	{
	}
	;

response_header_list:
	empty
	{
	}
	| response_header_list response_header
	{
	}
	;

response_header:
	response_header_cache_control
	{
	}
	| response_header_connection
	{
	}
	| response_header_date
	{
	}
	| response_header_pragma
	{
	}
	| response_header_trailer
	{
	}
	| response_header_transfer_encoding
	{
	}
	| response_header_upgrade
	{
	}
	| response_header_via
	{
	}
	| response_header_warning
	{
	}
	| response_header_mime_version
	{
	}
	| response_header_allow
	{
	}
	| response_header_content_encoding
	{
	}
	| response_header_content_language
	{
	}
	| response_header_content_length
	{
	}
	| response_header_content_location
	{
	}
	| response_header_content_md5
	{
	}
	| response_header_content_range
	{
	}
	| response_header_content_type
	{
	}
	| response_header_etag
	{
	}
	| response_header_expires
	{
	}
	| response_header_last_modified
	{
	}
	| response_header_content_base
	{
	}
	| response_header_content_version
	{
	}
	| response_header_derived_from
	{
	}
	| response_header_link
	{
	}
	| response_header_keep_alive
	{
	}
	| response_header_uri
	{
	}
	| response_header_accept_ranges
	{
	}
	| response_header_age
	{
	}
	| response_header_location
	{
	}
	| response_header_retry_after
	{
	}
	| response_header_server
	{
	}
	| response_header_vary
	{
	}
	| response_header_www_authenticate
	{
	}
	| response_header_set_cookie2
	{
	}
	| response_header_set_cookie
	{
	}
	| response_header_x_powered_by
	{
	}
	| response_header_proxy_authenticate
	{
	}
	;

response_header_cache_control:
	TOKEN_SERVER_HEADER_CACHE_CONTROL TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_connection:
	TOKEN_SERVER_HEADER_CONNECTION TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_date:
	TOKEN_SERVER_HEADER_DATE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_pragma:
	TOKEN_SERVER_HEADER_PRAGMA TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_trailer:
	TOKEN_SERVER_HEADER_TRAILER TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_transfer_encoding:
	TOKEN_SERVER_HEADER_TRANSFER_ENCODING TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_upgrade:
	TOKEN_SERVER_HEADER_UPGRADE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_via:
	TOKEN_SERVER_HEADER_VIA TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_warning:
	TOKEN_SERVER_HEADER_WARNING TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_mime_version:
	TOKEN_SERVER_HEADER_MIME_VERSION TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_allow:
	TOKEN_SERVER_HEADER_ALLOW TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_encoding:
	TOKEN_SERVER_HEADER_CONTENT_ENCODING TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_language:
	TOKEN_SERVER_HEADER_CONTENT_LANGUAGE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_length:
	TOKEN_SERVER_HEADER_CONTENT_LENGTH TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_location:
	TOKEN_SERVER_HEADER_CONTENT_LOCATION TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_md5:
	TOKEN_SERVER_HEADER_CONTENT_MD5 TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_range:
	TOKEN_SERVER_HEADER_CONTENT_RANGE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_type:
	TOKEN_SERVER_HEADER_CONTENT_TYPE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_etag:
	TOKEN_SERVER_HEADER_ETAG TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_expires:
	TOKEN_SERVER_HEADER_EXPIRES TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_last_modified:
	TOKEN_SERVER_HEADER_LAST_MODIFIED TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_base:
	TOKEN_SERVER_HEADER_CONTENT_BASE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_content_version:
	TOKEN_SERVER_HEADER_CONTENT_VERSION TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_derived_from:
	TOKEN_SERVER_HEADER_DERIVED_FROM TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_link:
	TOKEN_SERVER_HEADER_LINK TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_keep_alive:
	TOKEN_SERVER_HEADER_KEEP_ALIVE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_uri:
	TOKEN_SERVER_HEADER_URI TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_accept_ranges:
	TOKEN_SERVER_HEADER_ACCEPT_RANGES TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_age:
	TOKEN_SERVER_HEADER_AGE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_location:
	TOKEN_SERVER_HEADER_LOCATION TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_retry_after:
	TOKEN_SERVER_HEADER_RETRY_AFTER TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_server:
	TOKEN_SERVER_HEADER_SERVER TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_vary:
	TOKEN_SERVER_HEADER_VARY TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_www_authenticate:
	TOKEN_SERVER_HEADER_WWW_AUTHENTICATE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_set_cookie2:
	TOKEN_SERVER_HEADER_SET_COOKIE2 TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_set_cookie:
	TOKEN_SERVER_HEADER_SET_COOKIE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_x_powered_by:
	TOKEN_SERVER_HEADER_X_POWERED_BY TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_header_proxy_authenticate:
	TOKEN_SERVER_HEADER_PROXY_AUTHENTICATE TOKEN_SERVER_HEADER_COLON TOKEN_SERVER_HEADER_VALUE
	{
	}
	;

response_body:
	empty
	{
	}
	| response_body TOKEN_SERVER_BODY_PART
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
