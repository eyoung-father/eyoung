%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "http.h"
#include "http_private.h"
#include "http_client_lex.h"

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

int http_transaction_pair_id;

extern int http_client_lex_body_mode(yyscan_t scanner);
extern int http_client_body_lex(HTTP_CLIENT_STYPE *val, yyscan_t scanner);
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
%token TOKEN_CLIENT_FIRST_METHOD_UNKOWN

%token TOKEN_CLINET_FIRST_URI

%token TOKEN_CLIENT_FIRST_VERSION_09
%token TOKEN_CLIENT_FIRST_VERSION_10
%token TOKEN_CLIENT_FIRST_VERSION_11
%token TOKEN_CLIENT_FIRST_VERSION_UNKOWN

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

%token TOKEN_CLIENT_HEADER_VALUE
%token TOKEN_CLIENT_HEADER_TERM

%token TOKEN_CLIENT_BODY_PART
%token TOKEN_CLIENT_BODY_CHUNK_HEADER
%token TOKEN_CLIENT_BODY_CHUNK_TAILER
%token TOKEN_CLIENT_BODY_END

%union
{
	http_request_method_t method;
	http_version_t version;
	http_request_first_line_t *first_line;
	http_request_header_t *header;
	http_request_header_list_t header_list;
	http_body_t *body;
	http_request_t *request;
	http_string_t string;
	http_string_list_t string_list;
	http_chunk_body_part_t *chunk;
	http_chunk_body_list_t chunk_list;
	http_chunk_body_t chunk_body;
	http_chunk_body_header_t chunk_header;
}

%type <request>				request
%type <first_line>			request_line
%type <header_list>			request_headers
							request_header_list
%type <method>				request_line_method
%type <string>				request_line_uri
							request_header_value
							TOKEN_CLIENT_HEADER_VALUE
							TOKEN_CLINET_FIRST_URI
							TOKEN_CLIENT_BODY_PART
							TOKEN_CLIENT_BODY_CHUNK_TAILER
							TOKEN_CLIENT_BODY_CHUNK_HEADER
%type <version>				request_line_version
%type <body>				request_body
%type <string_list>			request_normal_body
							request_chunk_tailer_list
							request_chunk_data
%type <chunk>				request_chunk
%type <chunk_list>			request_chunk_list
%type <chunk_header>		request_chunk_header
%type <chunk_body>			request_chunk_body
%type <header>				request_header
							request_header_cache_control
							request_header_connection
							request_header_date
							request_header_pragma
							request_header_trailer
							request_header_transfer_encoding
							request_header_upgrade
							request_header_via
							request_header_warning
							request_header_mime_version
							request_header_allow
							request_header_content_encoding
							request_header_content_language
							request_header_content_length
							request_header_content_location
							request_header_content_md5
							request_header_content_range
							request_header_content_type
							request_header_etag
							request_header_expires
							request_header_last_modified
							request_header_content_base
							request_header_content_version
							request_header_derived_from
							request_header_link
							request_header_keep_alive
							request_header_uri
							request_header_accept_charset
							request_header_accept_encoding
							request_header_accept_language
							request_header_accept
							request_header_authorization
							request_header_except
							request_header_from
							request_header_host
							request_header_if_match
							request_header_if_modified_since
							request_header_if_none_match
							request_header_if_range
							request_header_if_unmodified_since
							request_header_max_forwards
							request_header_proxy_authorization
							request_header_range
							request_header_referer
							request_header_te
							request_header_user_agent
							request_header_cookie2
							request_header_cookie
							request_header_ua_pixels
							request_header_ua_color
							request_header_ua_os
							request_header_ua_cpu
							request_header_x_flash_version
							request_header_unkown

%destructor
{
	http_client_free_first_line(priv_decoder, $$);
}<first_line>

%destructor
{
	http_client_free_header(priv_decoder, $$);
}<header>

%destructor
{
	http_client_free_header_list(priv_decoder, &$$);
}<header_list>

%destructor
{
	http_client_free_request(priv_decoder, $$);
}<request>

%destructor
{
	http_client_free_string(priv_decoder, &$$);
}request_line_uri request_header_value

%destructor
{
	http_client_free_string_list(priv_decoder, &$$);
}<string_list>

%destructor
{
	http_client_free_body(priv_decoder, $$);
}<body>

%destructor
{
	http_client_free_chunk_body_part(priv_decoder, $$);
}<chunk>

%destructor
{
	http_client_free_chunk_body_list(priv_decoder, &$$);
}<chunk_list>

%destructor
{
	http_client_free_chunk_body(priv_decoder, &$$);
}<chunk_body>

%destructor
{
	http_client_free_string(priv_decoder, &$$.chunk_extension);
}<chunk_header>

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
		http_data_t *data = (http_data_t *)priv_data;
		STAILQ_INIT(&data->request_list);
	}
	| request_list request
	{
		http_data_t *data = (http_data_t *)priv_data;
		STAILQ_INSERT_TAIL(&data->request_list, $2, next);
	}
	TOKEN_CLIENT_BODY_END
	{
		/*do nothing*/
	}
	;

request: 
	request_line request_headers request_body
	{
		http_request_t *request = http_client_alloc_request(priv_decoder, $1, &$2, $3);
		if(!request)
		{
			http_debug(debug_http_client_parser, "failed to alloc request\n");
			YYABORT;
		}
		$$ = request;
	}
	;

request_line: 
	request_line_method request_line_uri request_line_version
	{
		http_request_first_line_t *first_line = http_client_alloc_first_line(priv_decoder, $1, &$2, $3);
		if(!first_line)
		{
			http_debug(debug_http_client_parser, "failed to alloc first line\n");
			YYABORT;
		}
		$$ = first_line;
	}
	;

request_line_method: 
	TOKEN_CLIENT_FIRST_METHOD_GET
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_GET;
	}
	| TOKEN_CLIENT_FIRST_METHOD_POST
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_POST;
	}
	| TOKEN_CLIENT_FIRST_METHOD_HEAD
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_HEAD;
	}
	| TOKEN_CLIENT_FIRST_METHOD_OPTIONS
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_OPTIONS;
	}
	| TOKEN_CLIENT_FIRST_METHOD_PUT
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_PUT;
	}
	| TOKEN_CLIENT_FIRST_METHOD_DELETE
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_DELETE;
	}
	| TOKEN_CLIENT_FIRST_METHOD_TRACE
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_TRACE;
	}
	| TOKEN_CLIENT_FIRST_METHOD_CONNECT
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_CONNECT;
	}
	| TOKEN_CLIENT_FIRST_METHOD_PATCH
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_PATCH;
	}
	| TOKEN_CLIENT_FIRST_METHOD_LINK
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_LINK;
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNLINK
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_UNLINK;
	}
	| TOKEN_CLIENT_FIRST_METHOD_PROPFIND
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_PROPFIND;
	}
	| TOKEN_CLIENT_FIRST_METHOD_PROPPATCH
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_PROPPATCH;
	}
	| TOKEN_CLIENT_FIRST_METHOD_MKCOL
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_MKCOL;
	}
	| TOKEN_CLIENT_FIRST_METHOD_COPY
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_COPY;
	}
	| TOKEN_CLIENT_FIRST_METHOD_MOVE
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_MOVE;
	}
	| TOKEN_CLIENT_FIRST_METHOD_LOCK
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_LOCK;
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNLOCK
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_UNLOCK;
	}
	| TOKEN_CLIENT_FIRST_METHOD_CHECKOUT
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_CHECKOUT;
	}
	| TOKEN_CLIENT_FIRST_METHOD_REPORT
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_REPORT;
	}
	| TOKEN_CLIENT_FIRST_METHOD_VERSION_CONTROL
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_VERSION_CONTROL;
	}
	| TOKEN_CLIENT_FIRST_METHOD_CHECKIN
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_CHECKIN;
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNCHECKOUT
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_UNCHECKOUT;
	}
	| TOKEN_CLIENT_FIRST_METHOD_MKWORKSPACE
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_MKWORKSPACE;
	}
	| TOKEN_CLIENT_FIRST_METHOD_UPDATE
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_UPDATE;
	}
	| TOKEN_CLIENT_FIRST_METHOD_LABEL
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_LABEL;
	}
	| TOKEN_CLIENT_FIRST_METHOD_MERGE
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_MERGE;
	}
	| TOKEN_CLIENT_FIRST_METHOD_MKACTIVITY
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_MKACTIVITY;
	}
	| TOKEN_CLIENT_FIRST_METHOD_ORDERPATCH
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_ORDERPATCH;
	}
	| TOKEN_CLIENT_FIRST_METHOD_BASELINE_CONTROL
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_BASELINE_CONTROL;
	}
	| TOKEN_CLIENT_FIRST_METHOD_UNKOWN
	{
		$$ = HTTP_REQUEST_METHOD_METHOD_UNKOWN;
	}
	;

request_line_uri: 
	TOKEN_CLINET_FIRST_URI
	{
		if(!http_client_alloc_string(priv_decoder, $1.buf, $1.len, &$$))
		{
			http_debug(debug_http_client_parser, "failed to duplicate uri value\n");
			YYABORT;
		}
	}
	;

request_line_version:
	TOKEN_CLIENT_FIRST_VERSION_09
	{
		$$ = HTTP_VERSION_09;
	}
	| TOKEN_CLIENT_FIRST_VERSION_10
	{
		$$ = HTTP_VERSION_10;
	}
	| TOKEN_CLIENT_FIRST_VERSION_11
	{
		$$ = HTTP_VERSION_11;
	}
	| TOKEN_CLIENT_FIRST_VERSION_UNKOWN
	{
		$$ = HTTP_VERSION_UNKOWN;
	}
	;

request_headers:
	request_header_list TOKEN_CLIENT_HEADER_TERM
	{
		STAILQ_INIT(&$$);
		STAILQ_CONCAT(&$$, &$1);
	}
	;

request_header_list:
	empty
	{
		STAILQ_INIT(&$$);
	}
	| request_header_list request_header
	{
		STAILQ_INSERT_TAIL(&$1, $2, next);
		STAILQ_CONCAT(&$$, &$1);
	}
	;

request_header:
	request_header_cache_control
	{
		$$ = $1;
	}
	| request_header_connection
	{
		$$ = $1;
	}
	| request_header_date
	{
		$$ = $1;
	}
	| request_header_pragma
	{
		$$ = $1;
	}
	| request_header_trailer
	{
		$$ = $1;
	}
	| request_header_transfer_encoding
	{
		$$ = $1;
	}
	| request_header_upgrade
	{
		$$ = $1;
	}
	| request_header_via
	{
		$$ = $1;
	}
	| request_header_warning
	{
		$$ = $1;
	}
	| request_header_mime_version
	{
		$$ = $1;
	}
	| request_header_allow
	{
		$$ = $1;
	}
	| request_header_content_encoding
	{
		$$ = $1;
	}
	| request_header_content_language
	{
		$$ = $1;
	}
	| request_header_content_length
	{
		$$ = $1;
	}
	| request_header_content_location
	{
		$$ = $1;
	}
	| request_header_content_md5
	{
		$$ = $1;
	}
	| request_header_content_range
	{
		$$ = $1;
	}
	| request_header_content_type
	{
		$$ = $1;
	}
	| request_header_etag
	{
		$$ = $1;
	}
	| request_header_expires
	{
		$$ = $1;
	}
	| request_header_last_modified
	{
		$$ = $1;
	}
	| request_header_content_base
	{
		$$ = $1;
	}
	| request_header_content_version
	{
		$$ = $1;
	}
	| request_header_derived_from
	{
		$$ = $1;
	}
	| request_header_link
	{
		$$ = $1;
	}
	| request_header_keep_alive
	{
		$$ = $1;
	}
	| request_header_uri
	{
		$$ = $1;
	}
	| request_header_accept_charset
	{
		$$ = $1;
	}
	| request_header_accept_encoding
	{
		$$ = $1;
	}
	| request_header_accept_language
	{
		$$ = $1;
	}
	| request_header_accept
	{
		$$ = $1;
	}
	| request_header_authorization
	{
		$$ = $1;
	}
	| request_header_except
	{
		$$ = $1;
	}
	| request_header_from
	{
		$$ = $1;
	}
	| request_header_host
	{
		$$ = $1;
	}
	| request_header_if_match
	{
		$$ = $1;
	}
	| request_header_if_modified_since
	{
		$$ = $1;
	}
	| request_header_if_none_match
	{
		$$ = $1;
	}
	| request_header_if_range
	{
		$$ = $1;
	}
	| request_header_if_unmodified_since
	{
		$$ = $1;
	}
	| request_header_max_forwards
	{
		$$ = $1;
	}
	| request_header_proxy_authorization
	{
		$$ = $1;
	}
	| request_header_range
	{
		$$ = $1;
	}
	| request_header_referer
	{
		$$ = $1;
	}
	| request_header_te
	{
		$$ = $1;
	}
	| request_header_user_agent
	{
		$$ = $1;
	}
	| request_header_cookie2
	{
		$$ = $1;
	}
	| request_header_cookie
	{
		$$ = $1;
	}
	| request_header_ua_pixels
	{
		$$ = $1;
	}
	| request_header_ua_color
	{
		$$ = $1;
	}
	| request_header_ua_os
	{
		$$ = $1;
	}
	| request_header_ua_cpu
	{
		$$ = $1;
	}
	| request_header_x_flash_version
	{
		$$ = $1;
	}
	| request_header_unkown
	{
		$$ = $1;
	}
	;

request_header_unkown:
	TOKEN_CLIENT_HEADER_UNKOWN request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_UNKOWN, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc UNKOWN header\n");
			YYABORT;
		}
		$$ = header;
	}
	;
	
request_header_cache_control:
	TOKEN_CLIENT_HEADER_CACHE_CONTROL request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CACHE_CONTROL, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Cache-Control header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_connection:
	TOKEN_CLIENT_HEADER_CONNECTION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONNECTION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Connection header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_date:
	TOKEN_CLIENT_HEADER_DATE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_DATE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Date header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_pragma:
	TOKEN_CLIENT_HEADER_PRAGMA request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_PRAGMA, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Pragma header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_trailer:
	TOKEN_CLIENT_HEADER_TRAILER request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_TRAILER, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Trailer header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_transfer_encoding:
	TOKEN_CLIENT_HEADER_TRANSFER_ENCODING request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_TRANSFER_ENCODING, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Transfer-Encoding header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_upgrade:
	TOKEN_CLIENT_HEADER_UPGRADE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_UPGRADE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Upgrade header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_via:
	TOKEN_CLIENT_HEADER_VIA request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_VIA, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Via header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_warning:
	TOKEN_CLIENT_HEADER_WARNING request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_WARNING, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Warning header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_mime_version:
	TOKEN_CLIENT_HEADER_MIME_VERSION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_MIME_VERSION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Mime-Version header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_allow:
	TOKEN_CLIENT_HEADER_ALLOW request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_ALLOW, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Allow header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_encoding:
	TOKEN_CLIENT_HEADER_CONTENT_ENCODING request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_ENCODING, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Encoding header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_language:
	TOKEN_CLIENT_HEADER_CONTENT_LANGUAGE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_LANGUAGE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Language header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_length:
	TOKEN_CLIENT_HEADER_CONTENT_LENGTH request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_LENGTH, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Length header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_location:
	TOKEN_CLIENT_HEADER_CONTENT_LOCATION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_LOCATION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Location header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_md5:
	TOKEN_CLIENT_HEADER_CONTENT_MD5 request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_MD5, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-MD5 header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_range:
	TOKEN_CLIENT_HEADER_CONTENT_RANGE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_RANGE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Range header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_type:
	TOKEN_CLIENT_HEADER_CONTENT_TYPE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_TYPE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Type header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_etag:
	TOKEN_CLIENT_HEADER_ETAG request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_ETAG, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Etag header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_expires:
	TOKEN_CLIENT_HEADER_EXPIRES request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_EXPIRES, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Expires header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_last_modified:
	TOKEN_CLIENT_HEADER_LAST_MODIFIED request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_LAST_MODIFIED, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Last-Modified header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_base:
	TOKEN_CLIENT_HEADER_CONTENT_BASE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_BASE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Base header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_content_version:
	TOKEN_CLIENT_HEADER_CONTENT_VERSION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_CONTENT_VERSION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Content-Version header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_derived_from:
	TOKEN_CLIENT_HEADER_DERIVED_FROM request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_DERIVED_FROM, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Derived-From header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_link:
	TOKEN_CLIENT_HEADER_LINK request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_LINK, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Link header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_keep_alive:
	TOKEN_CLIENT_HEADER_KEEP_ALIVE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_KEEP_ALIVE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Keep-Alive header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_uri:
	TOKEN_CLIENT_HEADER_URI request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_URI, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc URI header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_accept_charset:
	TOKEN_CLIENT_HEADER_ACCEPT_CHARSET request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_ACCEPT_CHARSET, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Accept-Charset header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_accept_encoding:
	TOKEN_CLIENT_HEADER_ACCEPT_ENCODING request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_ACCEPT_ENCODING, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Accept-Encoding header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_accept_language:
	TOKEN_CLIENT_HEADER_ACCEPT_LANGUAGE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_ACCEPT_LANGUAGE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Accept-Language header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_accept:
	TOKEN_CLIENT_HEADER_ACCEPT request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_ACCEPT, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Accept header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_authorization:
	TOKEN_CLIENT_HEADER_AUTHORIZATION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_AUTHORIZATION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc AUTHORIZATION header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_except:
	TOKEN_CLIENT_HEADER_EXCEPT request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_EXCEPT, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Except header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_from:
	TOKEN_CLIENT_HEADER_FROM request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_FROM, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc From header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_host:
	TOKEN_CLIENT_HEADER_HOST request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_HOST, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Host header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_if_match:
	TOKEN_CLIENT_HEADER_IF_MATCH request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_IF_MATCH, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc If-Match header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_if_modified_since:
	TOKEN_CLIENT_HEADER_IF_MODIFIED_SINCE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_IF_MODIFIED_SINCE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc If-Modified-Since header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_if_none_match:
	TOKEN_CLIENT_HEADER_IF_NONE_MATCH request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_IF_NONE_MATCH, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc If-None-Match header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_if_range:
	TOKEN_CLIENT_HEADER_IF_RANGE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_IF_RANGE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc If-Range header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_if_unmodified_since:
	TOKEN_CLIENT_HEADER_IF_UNMODIFIED_SINCE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_IF_UNMODIFIED_SINCE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc If-Unmodified-Since header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_max_forwards:
	TOKEN_CLIENT_HEADER_MAX_FORWARDS request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_MAX_FORWARDS, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Max-Forwards header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_proxy_authorization:
	TOKEN_CLIENT_HEADER_PROXY_AUTHORIZATION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_PROXY_AUTHORIZATION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc PROXY_AUTHORIZATION header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_range:
	TOKEN_CLIENT_HEADER_RANGE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_RANGE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Range header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_referer:
	TOKEN_CLIENT_HEADER_REFERER request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_REFERER, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Referer header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_te:
	TOKEN_CLIENT_HEADER_TE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_TE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Te header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_user_agent:
	TOKEN_CLIENT_HEADER_USER_AGENT request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_USER_AGENT, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc User-Agent header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_cookie2:
	TOKEN_CLIENT_HEADER_COOKIE2 request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_COOKIE2, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Cookie2 header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_cookie:
	TOKEN_CLIENT_HEADER_COOKIE request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_COOKIE, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc Cookie header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_ua_pixels:
	TOKEN_CLIENT_HEADER_UA_PIXELS request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_UA_PIXELS, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc UA-PIXELS header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_ua_color:
	TOKEN_CLIENT_HEADER_UA_COLOR request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_UA_COLOR, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc UA-COLOR header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_ua_os:
	TOKEN_CLIENT_HEADER_UA_OS request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_UA_OS, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc UA-OS header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_ua_cpu:
	TOKEN_CLIENT_HEADER_UA_CPU request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_UA_CPU, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc UA-CPU header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_x_flash_version:
	TOKEN_CLIENT_HEADER_X_FLASH_VERSION request_header_value
	{
		http_request_header_t *header = http_client_alloc_header(priv_decoder, HTTP_REQUEST_HEADER_X_FLASH_VERSION, &$2);
		if(!header)
		{
			http_debug(debug_http_client_parser, "failed to alloc X-FLASH-VERSION header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

request_header_value:
	empty
	{
		$$.buf = NULL;
		$$.len = 0;
	}
	| TOKEN_CLIENT_HEADER_VALUE
	{
		if(!http_client_alloc_string(priv_decoder, $1.buf, $1.len, &$$))
		{
			http_debug(debug_http_client_parser, "failed to duplicate header value\n");
			YYABORT;
		}
	}

request_body:
	empty
	{
		$$ = NULL;
	}
	| request_normal_body 
	{
		http_body_t *ret = http_client_alloc_body(priv_decoder);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc http normal body\n");
			YYABORT;
		}
		STAILQ_CONCAT(&ret->normal_body, &$1);
		$$ = ret;
	}
	| request_chunk_body
	{
		http_body_t *ret = http_client_alloc_body(priv_decoder);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc http chunk body\n");
			YYABORT;
		}
		STAILQ_CONCAT(&ret->chunk_body.chunk_list, &$1.chunk_list);
		STAILQ_CONCAT(&ret->chunk_body.chunk_tailer, &$1.chunk_tailer);
		$$ = ret;
	}
	;

request_normal_body:
	TOKEN_CLIENT_BODY_PART
	{
		http_string_t dup_part = {NULL, 0};
		if(!http_client_alloc_string(priv_decoder, $1.buf, $1.len, &dup_part))
		{
			http_debug(debug_http_client_parser, "failed to duplicate normal body part value\n");
			YYABORT;
		}

		http_string_list_part_t *ret = http_client_alloc_string_list_part(priv_decoder, &dup_part);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc normal body data part\n");
			http_client_free_string(priv_decoder, &dup_part);
			YYABORT;
		}
		STAILQ_INIT(&$$);
		STAILQ_INSERT_TAIL(&$$, ret, next);
	}
	| request_normal_body TOKEN_CLIENT_BODY_PART
	{
		http_string_t dup_part = {NULL, 0};
		if(!http_client_alloc_string(priv_decoder, $2.buf, $2.len, &dup_part))
		{
			http_debug(debug_http_client_parser, "failed to duplicate normal body part value\n");
			YYABORT;
		}

		http_string_list_part_t *ret = http_client_alloc_string_list_part(priv_decoder, &dup_part);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc normal body data part\n");
			http_client_free_string(priv_decoder, &dup_part);
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&$1, ret, next);
		STAILQ_CONCAT(&$$, &$1);
	}
	;

request_chunk_body:
	request_chunk_list request_chunk_tailer_list
	{
		STAILQ_INIT(&$$.chunk_list);
		STAILQ_INIT(&$$.chunk_tailer);

		STAILQ_CONCAT(&$$.chunk_list, &$1);
		STAILQ_CONCAT(&$$.chunk_tailer, &$2);
	}
	;

request_chunk_list:
	request_chunk
	{
		STAILQ_INIT(&$$);
		STAILQ_INSERT_TAIL(&$$, $1, next);
	}
	| request_chunk_list request_chunk
	{
		STAILQ_INSERT_TAIL(&$1, $2, next);
		STAILQ_CONCAT(&$$, &$1);
	}
	;

request_chunk:
	request_chunk_header request_chunk_data
	{
		http_chunk_body_part_t *ret = http_client_alloc_chunk_body_part(priv_decoder);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc chunk part\n");
			YYABORT;
		}
		ret->chunk_header = $1;
		STAILQ_CONCAT(&ret->chunk_value, &$2);
		$$ = ret;
	}
	;

request_chunk_header:
	TOKEN_CLIENT_BODY_CHUNK_HEADER
	{
		if(http_client_parse_chunk_header(priv_decoder, &$1, &$$))
		{
			http_debug(debug_http_client_parser, "failed to parse chunk header\n");
			YYABORT;
		}
	}
	;

request_chunk_data:
	empty
	{
		STAILQ_INIT(&$$);
	}
	| request_chunk_data TOKEN_CLIENT_BODY_PART
	{
		http_string_t dup_part = {NULL, 0};
		if(!http_client_alloc_string(priv_decoder, $2.buf, $2.len, &dup_part))
		{
			http_debug(debug_http_client_parser, "failed to duplicate chunk body part value\n");
			YYABORT;
		}

		http_string_list_part_t *ret = http_client_alloc_string_list_part(priv_decoder, &dup_part);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc chunk data part\n");
			http_client_free_string(priv_decoder, &dup_part);
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&$1, ret, next);
		STAILQ_CONCAT(&$$, &$1);
	}
	;

request_chunk_tailer_list:
	empty
	{
		STAILQ_INIT(&$$);
	}
	| request_chunk_tailer_list TOKEN_CLIENT_BODY_CHUNK_TAILER
	{
		http_string_t dup_part = {NULL, 0};
		if(!http_client_alloc_string(priv_decoder, $2.buf, $2.len, &dup_part))
		{
			http_debug(debug_http_client_parser, "failed to duplicate chunk tailer\n");
			YYABORT;
		}

		http_string_list_part_t *ret = http_client_alloc_string_list_part(priv_decoder, &dup_part);
		if(!ret)
		{
			http_debug(debug_http_client_parser, "failed to alloc chunk tailer part\n");
			http_client_free_string(priv_decoder, &dup_part);
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&$1, ret, next);
		STAILQ_CONCAT(&$$, &$1);
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
		if(http_client_lex_body_mode(lexier))
			token = http_client_body_lex(&value, lexier);
		else
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
			http_debug(debug_http_client_parser, "client event id of %s is %d\n", name, yytid[YYNTOKENS + index]);
		else
			http_debug(debug_http_client_parser, "failed to register client event %s\n", name);
	}

	http_transaction_pair_id = ey_engine_find_event(engine, "transaction_pair");
}
#undef priv_decoder
