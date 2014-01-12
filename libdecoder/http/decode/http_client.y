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
#define YY_REDUCTION_CALLBACK(data,name,id,val)					\
	do															\
	{															\
		if(http_element_detect(data,name,id,val,				\
			cluster_buffer,cluster_buffer_len)<0)				\
		{														\
			http_debug(debug_http_detect, "find attack!\n");	\
			return -1;											\
		}														\
	}while(0)

#define priv_decoder											\
	((http_decoder_t*)(((http_data_t*)priv_data)->decoder))

int http_cmd_pair_id;
%}
%token TOKEN_CLIENT_CONTINUE

%token TOKEN_CLIENT_FIRST_METHOD_GET
%token TOKEN_CLIENT_FIRST_METHOD_POST
%token TOKEN_CLIENT_FIRST_METHOD_HEAD
%token TOKEN_CLIENT_FIRST_METHOD_OPTIONS
%token TOKEN_CLIENT_FIRST_METHOD_PUT
%token TOKEN_CLIENT_FIRST_METHOD_DELETE
%token TOKEN_CLIENT_FIRST_METHOD_TRACE
%token TOKEN_CLIENT_FIRST_METHOD_CONNECT
%token TOKEN_CLIENT_FIRST_METHOD_PATCH
%token TOKEN_CLIENT_FIRST_METHOD_LINK
%token TOKEN_CLIENT_FIRST_METHOD_UNLINK
%token TOKEN_CLIENT_FIRST_METHOD_PROPFIND
%token TOKEN_CLIENT_FIRST_METHOD_PROPPATCH
%token TOKEN_CLIENT_FIRST_METHOD_MKCOL
%token TOKEN_CLIENT_FIRST_METHOD_COPY
%token TOKEN_CLIENT_FIRST_METHOD_MOVE
%token TOKEN_CLIENT_FIRST_METHOD_LOCK
%token TOKEN_CLIENT_FIRST_METHOD_UNLOCK
%token TOKEN_CLIENT_FIRST_METHOD_CHECKOUT
%token TOKEN_CLIENT_FIRST_METHOD_REPORT
%token TOKEN_CLIENT_FIRST_METHOD_VERSION_CONTROL
%token TOKEN_CLIENT_FIRST_METHOD_CHECKIN
%token TOKEN_CLIENT_FIRST_METHOD_UNCHECKOUT
%token TOKEN_CLIENT_FIRST_METHOD_MKWORKSPACE
%token TOKEN_CLIENT_FIRST_METHOD_UPDATE
%token TOKEN_CLIENT_FIRST_METHOD_LABEL
%token TOKEN_CLIENT_FIRST_METHOD_MERGE
%token TOKEN_CLIENT_FIRST_METHOD_MKACTIVITY
%token TOKEN_CLIENT_FIRST_METHOD_ORDERPATCH
%token TOKEN_CLIENT_FIRST_METHOD_BASELINE_CONTROL

%token TOKEN_CLINET_FIRST_URI

%token TOKEN_CLIENT_FIRST_VERSION_09
%token TOKEN_CLIENT_FIRST_VERSION_10
%token TOKEN_CLIENT_FIRST_VERSION_11

%token TOKEN_CLIENT_HEADER_HOST
%token TOKEN_CLIENT_HEADER_CACHE_CONTROL	
%token TOKEN_CLIENT_HEADER_CONNECTION
%token TOKEN_CLIENT_HEADER_DATE
%token TOKEN_CLIENT_HEADER_PRAGMA
%token TOKEN_CLIENT_HEADER_TRAILER 
%token TOKEN_CLIENT_HEADER_TRANSFER_ENCODING
%token TOKEN_CLIENT_HEADER_UPGRADE
%token TOKEN_CLIENT_HEADER_VIA
%token TOKEN_CLIENT_HEADER_WARNING
%token TOKEN_CLIENT_HEADER_MIME_VERSION
%token TOKEN_CLIENT_HEADER_ALLOW
%token TOKEN_CLIENT_HEADER_CONTENT_ENCODING
%token TOKEN_CLIENT_HEADER_CONTENT_LANGUAGE
%token TOKEN_CLIENT_HEADER_CONTENT_LENGTH
%token TOKEN_CLIENT_HEADER_CONTENT_LOCATION
%token TOKEN_CLIENT_HEADER_CONTENT_MD5
%token TOKEN_CLIENT_HEADER_CONTENT_RANGE
%token TOKEN_CLIENT_HEADER_CONTENT_TYPE
%token TOKEN_CLIENT_HEADER_ETAG
%token TOKEN_CLIENT_HEADER_EXPIRES
%token TOKEN_CLIENT_HEADER_LAST_MODIFIED
%token TOKEN_CLIENT_HEADER_CONTENT_BASE
%token TOKEN_CLIENT_HEADER_CONTENT_VERSION
%token TOKEN_CLIENT_HEADER_DERIVED_FROM
%token TOKEN_CLIENT_HEADER_LINK
%token TOKEN_CLIENT_HEADER_KEEP_ALIVE
%token TOKEN_CLIENT_HEADER_URI
%token TOKEN_CLIENT_HEADER_ACCEPT_CHARSET
%token TOKEN_CLIENT_HEADER_ACCEPT_ENCODING
%token TOKEN_CLIENT_HEADER_ACCEPT_LANGUAGE
%token TOKEN_CLIENT_HEADER_ACCEPT
%token TOKEN_CLIENT_HEADER_AUTHORIZATION
%token TOKEN_CLIENT_HEADER_EXCEPT
%token TOKEN_CLIENT_HEADER_FROM
%token TOKEN_CLIENT_HEADER_IF_MATCH
%token TOKEN_CLIENT_HEADER_IF_MODIFIED_SINCE
%token TOKEN_CLIENT_HEADER_IF_NONE_MATCH
%token TOKEN_CLIENT_HEADER_IF_RANGE
%token TOKEN_CLIENT_HEADER_IF_UNMODIFIED_SINCE
%token TOKEN_CLIENT_HEADER_MAX_FORWARDS
%token TOKEN_CLIENT_HEADER_RANGE
%token TOKEN_CLIENT_HEADER_REFERER
%token TOKEN_CLIENT_HEADER_TE
%token TOKEN_CLIENT_HEADER_USER_AGENT
%token TOKEN_CLIENT_HEADER_COOKIE2
%token TOKEN_CLIENT_HEADER_COOKIE
%token TOKEN_CLIENT_HEADER_UA_PIXELS
%token TOKEN_CLIENT_HEADER_UA_COLOR
%token TOKEN_CLIENT_HEADER_UA_OS
%token TOKEN_CLIENT_HEADER_UA_CPU
%token TOKEN_CLIENT_HEADER_X_FLASH_VERSION
%token TOKEN_CLIENT_HEADER_PROXY_AUTHORIZATION
%token TOKEN_CLIENT_HEADER_UNKOWN

%token TOKEN_CLIENT_HEADER_COLON
%token TOKEN_CLIENT_HEADER_VALUE
%token TOKEN_CLIENT_HEADER_TERM

%token TOKEN_CLIENT_BODY_PART

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
	empty
	{
	}
	| request_list request
	{
	}
	;

request: 
	request_line request_headers request_body
	{
	}
	;

request_line: 
	request_line_method request_line_uri request_line_version
	{
	}
	;

request_line_method: 
	TOKEN_CLIENT_FIRST_METHOD_GET
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_POST
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_HEAD
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_OPTIONS
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_PUT
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_DELETE
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_TRACE
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_CONNECT
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_PATCH
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_LINK
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNLINK
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_PROPFIND
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_PROPPATCH
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_MKCOL
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_COPY
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_MOVE
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_LOCK
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNLOCK
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_CHECKOUT
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_REPORT
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_VERSION_CONTROL
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_CHECKIN
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNCHECKOUT
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_MKWORKSPACE
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_UPDATE
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_LABEL
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_MERGE
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_MKACTIVITY
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_ORDERPATCH
	{
	}
	| TOKEN_CLIENT_FIRST_METHOD_BASELINE_CONTROL
	{
	}
	| error
	{
	}
	;

request_line_uri: 
	TOKEN_CLINET_FIRST_URI
	{
	}
	;

request_line_version:
	TOKEN_CLIENT_FIRST_VERSION_09
	{
	}
	| TOKEN_CLIENT_FIRST_VERSION_10
	{
	}
	| TOKEN_CLIENT_FIRST_VERSION_11
	{
	}
	| error
	{
	}
	;

request_headers:
	request_header_list TOKEN_CLIENT_HEADER_TERM
	{
	}
	;

request_header_list:
	empty
	{
	}
	| request_header_list request_header
	{
	}
	;

request_header:
	request_header_cache_control
	{
	}
	| request_header_connection
	{
	}
	| request_header_date
	{
	}
	| request_header_pragma
	{
	}
	| request_header_trailer
	{
	}
	| request_header_transfer_encoding
	{
	}
	| request_header_upgrade
	{
	}
	| request_header_via
	{
	}
	| request_header_warning
	{
	}
	| request_header_mime_version
	{
	}
	| request_header_allow
	{
	}
	| request_header_content_encoding
	{
	}
	| request_header_content_language
	{
	}
	| request_header_content_length
	{
	}
	| request_header_content_location
	{
	}
	| request_header_content_md5
	{
	}
	| request_header_content_range
	{
	}
	| request_header_content_type
	{
	}
	| request_header_etag
	{
	}
	| request_header_expires
	{
	}
	| request_header_last_modified
	{
	}
	| request_header_content_base
	{
	}
	| request_header_content_version
	{
	}
	| request_header_derived_from
	{
	}
	| request_header_link
	{
	}
	| request_header_keep_alive
	{
	}
	| request_header_uri
	{
	}
	| request_header_accept_charset
	{
	}
	| request_header_accept_encoding
	{
	}
	| request_header_accept_language
	{
	}
	| request_header_accept
	{
	}
	| request_header_authorization
	{
	}
	| request_header_except
	{
	}
	| request_header_from
	{
	}
	| request_header_host
	{
	}
	| request_header_if_match
	{
	}
	| request_header_if_modified_since
	{
	}
	| request_header_if_none_match
	{
	}
	| request_header_if_range
	{
	}
	| request_header_if_unmodified_since
	{
	}
	| request_header_max_forwards
	{
	}
	| request_header_proxy_authorization
	{
	}
	| request_header_range
	{
	}
	| request_header_referer
	{
	}
	| request_header_te
	{
	}
	| request_header_user_agent
	{
	}
	| request_header_cookie2
	{
	}
	| request_header_cookie
	{
	}
	| request_header_ua_pixels
	{
	}
	| request_header_ua_color
	{
	}
	| request_header_ua_os
	{
	}
	| request_header_ua_cpu
	{
	}
	| request_header_x_flash_version
	{
	}
	;

request_header_cache_control:
	TOKEN_CLIENT_HEADER_CACHE_CONTROL TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_connection:
	TOKEN_CLIENT_HEADER_CONNECTION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_date:
	TOKEN_CLIENT_HEADER_DATE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_pragma:
	TOKEN_CLIENT_HEADER_PRAGMA TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_trailer:
	TOKEN_CLIENT_HEADER_TRAILER TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_transfer_encoding:
	TOKEN_CLIENT_HEADER_TRANSFER_ENCODING TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_upgrade:
	TOKEN_CLIENT_HEADER_UPGRADE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_via:
	TOKEN_CLIENT_HEADER_VIA TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_warning:
	TOKEN_CLIENT_HEADER_WARNING TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_mime_version:
	TOKEN_CLIENT_HEADER_MIME_VERSION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_allow:
	TOKEN_CLIENT_HEADER_ALLOW TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_encoding:
	TOKEN_CLIENT_HEADER_CONTENT_ENCODING TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_language:
	TOKEN_CLIENT_HEADER_CONTENT_LANGUAGE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_length:
	TOKEN_CLIENT_HEADER_CONTENT_LENGTH TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_location:
	TOKEN_CLIENT_HEADER_CONTENT_LOCATION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_md5:
	TOKEN_CLIENT_HEADER_CONTENT_MD5 TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_range:
	TOKEN_CLIENT_HEADER_CONTENT_RANGE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_type:
	TOKEN_CLIENT_HEADER_CONTENT_TYPE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_etag:
	TOKEN_CLIENT_HEADER_ETAG TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_expires:
	TOKEN_CLIENT_HEADER_EXPIRES TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_last_modified:
	TOKEN_CLIENT_HEADER_LAST_MODIFIED TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_base:
	TOKEN_CLIENT_HEADER_CONTENT_BASE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_content_version:
	TOKEN_CLIENT_HEADER_CONTENT_VERSION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_derived_from:
	TOKEN_CLIENT_HEADER_DERIVED_FROM TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_link:
	TOKEN_CLIENT_HEADER_LINK TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_keep_alive:
	TOKEN_CLIENT_HEADER_KEEP_ALIVE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_uri:
	TOKEN_CLIENT_HEADER_URI TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_accept_charset:
	TOKEN_CLIENT_HEADER_ACCEPT_CHARSET TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_accept_encoding:
	TOKEN_CLIENT_HEADER_ACCEPT_ENCODING TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_accept_language:
	TOKEN_CLIENT_HEADER_ACCEPT_LANGUAGE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_accept:
	TOKEN_CLIENT_HEADER_ACCEPT TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_authorization:
	TOKEN_CLIENT_HEADER_AUTHORIZATION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_except:
	TOKEN_CLIENT_HEADER_EXCEPT TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_from:
	TOKEN_CLIENT_HEADER_FROM TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_host:
	TOKEN_CLIENT_HEADER_HOST TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_if_match:
	TOKEN_CLIENT_HEADER_IF_MATCH TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_if_modified_since:
	TOKEN_CLIENT_HEADER_IF_MODIFIED_SINCE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_if_none_match:
	TOKEN_CLIENT_HEADER_IF_NONE_MATCH TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_if_range:
	TOKEN_CLIENT_HEADER_IF_RANGE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_if_unmodified_since:
	TOKEN_CLIENT_HEADER_IF_UNMODIFIED_SINCE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_max_forwards:
	TOKEN_CLIENT_HEADER_MAX_FORWARDS TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_proxy_authorization:
	TOKEN_CLIENT_HEADER_PROXY_AUTHORIZATION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_range:
	TOKEN_CLIENT_HEADER_RANGE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_referer:
	TOKEN_CLIENT_HEADER_REFERER TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_te:
	TOKEN_CLIENT_HEADER_TE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_user_agent:
	TOKEN_CLIENT_HEADER_USER_AGENT TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_cookie2:
	TOKEN_CLIENT_HEADER_COOKIE2 TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_cookie:
	TOKEN_CLIENT_HEADER_COOKIE TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_ua_pixels:
	TOKEN_CLIENT_HEADER_UA_PIXELS TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_ua_color:
	TOKEN_CLIENT_HEADER_UA_COLOR TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_ua_os:
	TOKEN_CLIENT_HEADER_UA_OS TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_ua_cpu:
	TOKEN_CLIENT_HEADER_UA_CPU TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;

request_header_x_flash_version:
	TOKEN_CLIENT_HEADER_X_FLASH_VERSION TOKEN_CLIENT_HEADER_COLON TOKEN_CLIENT_HEADER_VALUE
	{
	}
	;


request_body:
	empty
	{
	}
	| request_body TOKEN_CLIENT_BODY_PART
	{
	}
	;

empty:
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
