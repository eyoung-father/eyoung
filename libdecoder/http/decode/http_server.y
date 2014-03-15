%{
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "http.h"
#include "http_private.h"
#include "http_server_lex.h"
#include "ey_zlib.h"
#include "html.h"

#ifdef YY_REDUCTION_CALLBACK
#undef YY_REDUCTION_CALLBACK
#endif
#define YY_REDUCTION_CALLBACK(data,name,id,val)									\
	do																			\
	{																			\
		if(http_element_detect(data,name,id,val) < 0)							\
		{																		\
			http_debug(debug_http_detect, "find attack!\n");					\
			return -1;															\
		}																		\
	}while(0)

#define priv_decoder															\
	((http_decoder_t*)(((http_data_t*)priv_data)->decoder))

extern int http_server_lex_body_mode(yyscan_t scanner);
extern int http_server_body_lex(HTTP_SERVER_STYPE *val, yyscan_t scanner);
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
%token TOKEN_SERVER_HEADER_UNKOWN

%token TOKEN_SERVER_FIRST_VERSION_09
%token TOKEN_SERVER_FIRST_VERSION_10
%token TOKEN_SERVER_FIRST_VERSION_11
%token TOKEN_SERVER_FIRST_VERSION_UNKOWN

%token TOKEN_SERVER_FIRST_CODE
%token TOKEN_SERVER_FIRST_VALUE

%token TOKEN_SERVER_HEADER_VALUE
%token TOKEN_SERVER_HEADER_TERM

%token TOKEN_SERVER_BODY_PART
%token TOKEN_SERVER_BODY_CHUNK_HEADER
%token TOKEN_SERVER_BODY_CHUNK_TAILER
%token TOKEN_SERVER_BODY_END

%union
{
	http_version_t version;
	http_response_code_t code;
	http_string_t string;
	http_string_list_t string_list;
	http_response_first_line_t *first_line;
	http_response_header_t *header;
	http_response_header_list_t header_list;
	http_body_t *body;
	http_response_t *response;
	http_chunk_body_part_t *chunk;
	http_chunk_body_list_t chunk_list;
	http_chunk_body_t chunk_body;
	http_chunk_body_header_t chunk_header;
	http_body_info_t body_info;
}

%type <response>		response
%type <first_line>		response_line
%type <header_list>		response_headers
						response_header_list
%type <body>			response_body
%type <string_list>		response_normal_body
						response_chunk_tailer_list
						response_chunk_data
%type <chunk>			response_chunk
%type <chunk_list>		response_chunk_list
%type <chunk_header>	response_chunk_header
%type <chunk_body>		response_chunk_body
%type <version>			response_line_version
%type <code>			response_line_code
						TOKEN_SERVER_FIRST_CODE
%type <string>			response_line_message
						response_header_value
						response_body_part
						TOKEN_SERVER_FIRST_VALUE
						TOKEN_SERVER_HEADER_VALUE
						TOKEN_SERVER_BODY_PART
						TOKEN_SERVER_BODY_CHUNK_TAILER
						TOKEN_SERVER_BODY_CHUNK_HEADER
%type <header>			response_header
						response_header_cache_control
						response_header_connection
						response_header_date
						response_header_pragma
						response_header_trailer
						response_header_transfer_encoding
						response_header_upgrade
						response_header_via
						response_header_warning
						response_header_mime_version
						response_header_allow
						response_header_content_encoding
						response_header_content_language
						response_header_content_length
						response_header_content_location
						response_header_content_md5
						response_header_content_range
						response_header_content_type
						response_header_etag
						response_header_expires
						response_header_last_modified
						response_header_content_base
						response_header_content_version
						response_header_derived_from
						response_header_link
						response_header_keep_alive
						response_header_uri
						response_header_accept_ranges
						response_header_age
						response_header_location
						response_header_retry_after
						response_header_server
						response_header_vary
						response_header_www_authenticate
						response_header_set_cookie2
						response_header_set_cookie
						response_header_x_powered_by
						response_header_proxy_authenticate
						response_header_unkown

%destructor
{
	http_server_free_response(priv_decoder, $$);
}<response>

%destructor
{
	http_server_free_first_line(priv_decoder, $$);
}<first_line>

%destructor
{
	http_server_free_header_list(priv_decoder, &$$);
}<header_list>

%destructor
{
	http_server_free_string(priv_decoder, &$$);
}response_line_message response_header_value

%destructor
{
	http_server_free_string_list(priv_decoder, &$$);
}<string_list>

%destructor
{
	http_server_free_header(priv_decoder, $$);
}<header>

%destructor
{
	http_server_free_chunk_body_part(priv_decoder, $$);
}<chunk>

%destructor
{
	http_server_free_chunk_body_list(priv_decoder, &$$);
}<chunk_list>

%destructor
{
	http_server_free_chunk_body(priv_decoder, &$$);
}<chunk_body>

%destructor
{
	http_server_free_string(priv_decoder, &$$.chunk_extension);
}<chunk_header>

%destructor
{
	http_server_free_body(priv_decoder, $$);
}<body>

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
	response_empty
	{
		http_data_t *data = (http_data_t *)priv_data;
		STAILQ_INIT(&data->response_list);
	}
	| response_list response
	{
		http_data_t *data = (http_data_t *)priv_data;
		STAILQ_INSERT_TAIL(&data->response_list, $2, next);

		int r = http_add_transaction(priv_decoder, data);
		if(r<0)
		{
			http_debug(debug_http_detect, "find attack in http transaction\n");
			return -1;
		}
		else if(r>0)
		{
			http_debug(debug_http_mem, "add http transaction failed\n");
			return -1;
		}
		else
		{
			http_debug(debug_http_server_parser, "add http transaction successfully\n");
		}
	}
	TOKEN_SERVER_BODY_END
	{
		/*do nothing*/
	}
	;

response_empty:
	{
	}
	;

response:
	response_line response_headers response_body
	{
		http_response_t *response = http_server_alloc_response(priv_decoder, $1, &$2, $3);
		if(!response)
		{
			http_debug(debug_http_server_parser, "failed to alloc response\n");
			YYABORT;
		}
		$$ = response;
	}
	;

response_line:
	response_line_version response_line_code response_line_message
	{
		http_response_first_line_t *first_line = http_server_alloc_first_line(priv_decoder, $1, $2, &$3);
		if(!first_line)
		{
			http_debug(debug_http_server_parser, "failed to alloc first line\n");
			YYABORT;
		}
		$$ = first_line;
	}
	;

response_line_version:
	TOKEN_SERVER_FIRST_VERSION_09
	{
		$$ = HTTP_VERSION_09;
	}
	| TOKEN_SERVER_FIRST_VERSION_10
	{
		$$ = HTTP_VERSION_10;
	}
	| TOKEN_SERVER_FIRST_VERSION_11
	{
		$$ = HTTP_VERSION_11;
	}
	| TOKEN_SERVER_FIRST_VERSION_UNKOWN
	{
		$$ = HTTP_VERSION_UNKOWN;
	}
	;

response_line_code:
	TOKEN_SERVER_FIRST_CODE
	{
		$$ = $1;
	}
	;

response_line_message:
	TOKEN_SERVER_FIRST_VALUE
	{
		http_string_t dup_string = {NULL, 0};
		if(!http_server_alloc_string(priv_decoder, $1.buf, $1.len, &dup_string))
		{
			http_debug(debug_http_server_parser, "alloc first line failed\n");
			YYABORT;
		}
		$$ = dup_string;
	}
	;

response_headers:
	response_header_list TOKEN_SERVER_HEADER_TERM
	{
		STAILQ_INIT(&$$);
		STAILQ_CONCAT(&$$, &$1);
	}
	;

response_header_list:
	response_empty
	{
		STAILQ_INIT(&$$);
	}
	| response_header_list response_header
	{
		$$ = $1;
		STAILQ_INSERT_TAIL(&$$, $2, next);
	}
	;

response_header:
	response_header_cache_control
	{
		$$ = $1;
	}
	| response_header_connection
	{
		$$ = $1;
	}
	| response_header_date
	{
		$$ = $1;
	}
	| response_header_pragma
	{
		$$ = $1;
	}
	| response_header_trailer
	{
		$$ = $1;
	}
	| response_header_transfer_encoding
	{
		$$ = $1;
	}
	| response_header_upgrade
	{
		$$ = $1;
	}
	| response_header_via
	{
		$$ = $1;
	}
	| response_header_warning
	{
		$$ = $1;
	}
	| response_header_mime_version
	{
		$$ = $1;
	}
	| response_header_allow
	{
		$$ = $1;
	}
	| response_header_content_encoding
	{
		$$ = $1;
	}
	| response_header_content_language
	{
		$$ = $1;
	}
	| response_header_content_length
	{
		$$ = $1;
	}
	| response_header_content_location
	{
		$$ = $1;
	}
	| response_header_content_md5
	{
		$$ = $1;
	}
	| response_header_content_range
	{
		$$ = $1;
	}
	| response_header_content_type
	{
		$$ = $1;
	}
	| response_header_etag
	{
		$$ = $1;
	}
	| response_header_expires
	{
		$$ = $1;
	}
	| response_header_last_modified
	{
		$$ = $1;
	}
	| response_header_content_base
	{
		$$ = $1;
	}
	| response_header_content_version
	{
		$$ = $1;
	}
	| response_header_derived_from
	{
		$$ = $1;
	}
	| response_header_link
	{
		$$ = $1;
	}
	| response_header_keep_alive
	{
		$$ = $1;
	}
	| response_header_uri
	{
		$$ = $1;
	}
	| response_header_accept_ranges
	{
		$$ = $1;
	}
	| response_header_age
	{
		$$ = $1;
	}
	| response_header_location
	{
		$$ = $1;
	}
	| response_header_retry_after
	{
		$$ = $1;
	}
	| response_header_server
	{
		$$ = $1;
	}
	| response_header_vary
	{
		$$ = $1;
	}
	| response_header_www_authenticate
	{
		$$ = $1;
	}
	| response_header_set_cookie2
	{
		$$ = $1;
	}
	| response_header_set_cookie
	{
		$$ = $1;
	}
	| response_header_x_powered_by
	{
		$$ = $1;
	}
	| response_header_proxy_authenticate
	{
		$$ = $1;
	}
	| response_header_unkown
	{
		$$ = $1;
	}
	;

response_header_unkown:
	TOKEN_SERVER_HEADER_UNKOWN response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_UNKOWN, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc UNKOWN header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_cache_control:
	TOKEN_SERVER_HEADER_CACHE_CONTROL response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CACHE_CONTROL, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Cache-Control header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_connection:
	TOKEN_SERVER_HEADER_CONNECTION response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONNECTION, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Connection header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_date:
	TOKEN_SERVER_HEADER_DATE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_DATE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Date header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_pragma:
	TOKEN_SERVER_HEADER_PRAGMA response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_PRAGMA, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Pragma header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_trailer:
	TOKEN_SERVER_HEADER_TRAILER response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_TRAILER, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Trailer header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_transfer_encoding:
	TOKEN_SERVER_HEADER_TRANSFER_ENCODING response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_TRANSFER_ENCODING, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Transfer-Encoding header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_upgrade:
	TOKEN_SERVER_HEADER_UPGRADE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_UPGRADE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Upgrade header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_via:
	TOKEN_SERVER_HEADER_VIA response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_VIA, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Via header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_warning:
	TOKEN_SERVER_HEADER_WARNING response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_WARNING, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Warning header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_mime_version:
	TOKEN_SERVER_HEADER_MIME_VERSION response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_MIME_VERSION, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Mime-Version header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_allow:
	TOKEN_SERVER_HEADER_ALLOW response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_ALLOW, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Allow header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_encoding:
	TOKEN_SERVER_HEADER_CONTENT_ENCODING response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_ENCODING, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Encoding header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_language:
	TOKEN_SERVER_HEADER_CONTENT_LANGUAGE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Language header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_length:
	TOKEN_SERVER_HEADER_CONTENT_LENGTH response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_LENGTH, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Length header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_location:
	TOKEN_SERVER_HEADER_CONTENT_LOCATION response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_LOCATION, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Location header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_md5:
	TOKEN_SERVER_HEADER_CONTENT_MD5 response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_MD5, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-MD5 header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_range:
	TOKEN_SERVER_HEADER_CONTENT_RANGE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_RANGE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Range header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_type:
	TOKEN_SERVER_HEADER_CONTENT_TYPE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_TYPE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Type header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_etag:
	TOKEN_SERVER_HEADER_ETAG response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_ETAG, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Etag header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_expires:
	TOKEN_SERVER_HEADER_EXPIRES response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_EXPIRES, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Expires header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_last_modified:
	TOKEN_SERVER_HEADER_LAST_MODIFIED response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_LAST_MODIFIED, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Last-Modified header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_base:
	TOKEN_SERVER_HEADER_CONTENT_BASE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_BASE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Base header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_content_version:
	TOKEN_SERVER_HEADER_CONTENT_VERSION response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_CONTENT_VERSION, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Content-Version header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_derived_from:
	TOKEN_SERVER_HEADER_DERIVED_FROM response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_DERIVED_FROM, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Derived-From header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_link:
	TOKEN_SERVER_HEADER_LINK response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_LINK, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Link header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_keep_alive:
	TOKEN_SERVER_HEADER_KEEP_ALIVE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_KEEP_ALIVE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Keep-Alive header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_uri:
	TOKEN_SERVER_HEADER_URI response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_URI, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc URI header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_accept_ranges:
	TOKEN_SERVER_HEADER_ACCEPT_RANGES response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_ACCEPT_RANGES, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Accept-Range header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_age:
	TOKEN_SERVER_HEADER_AGE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_AGE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Age header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_location:
	TOKEN_SERVER_HEADER_LOCATION response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_LOCATION, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Location header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_retry_after:
	TOKEN_SERVER_HEADER_RETRY_AFTER response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_RETRY_AFTER, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Retry-After header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_server:
	TOKEN_SERVER_HEADER_SERVER response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_SERVER, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Server header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_vary:
	TOKEN_SERVER_HEADER_VARY response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_VARY, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Vary header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_www_authenticate:
	TOKEN_SERVER_HEADER_WWW_AUTHENTICATE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc WWW-AUTHENTICATE header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_set_cookie2:
	TOKEN_SERVER_HEADER_SET_COOKIE2 response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_SET_COOKIE2, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Set-Cookie2 header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_set_cookie:
	TOKEN_SERVER_HEADER_SET_COOKIE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_SET_COOKIE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Set-Cookie header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_x_powered_by:
	TOKEN_SERVER_HEADER_X_POWERED_BY response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_X_POWERED_BY, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc X-Powered-By header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_proxy_authenticate:
	TOKEN_SERVER_HEADER_PROXY_AUTHENTICATE response_header_value
	{
		http_response_header_t *header = http_server_alloc_header(priv_decoder, HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE, &$2);
		if(!header)
		{
			http_debug(debug_http_server_parser, "failed to alloc Proxy-Authenticate header\n");
			YYABORT;
		}
		$$ = header;
	}
	;

response_header_value:
	response_empty
	{
		$$.buf = NULL;
		$$.len = 0;
	}
	| TOKEN_SERVER_HEADER_VALUE
	{
		http_string_t dup_string = {NULL, 0};
		if(!http_server_alloc_string(priv_decoder, $1.buf, $1.len, &dup_string))
		{
			http_debug(debug_http_server_parser, "alloc header value failed\n");
			YYABORT;
		}
		$$ = dup_string;
	}
	;

response_body:
	response_empty
	{
		http_data_t *data = (http_data_t *)priv_data;
		http_parser_t *parser = &data->response_parser;

		parser->content_encoding = HTTP_BODY_CONTENT_ENCODING_UNKOWN;
		parser->content_maintype = HTTP_BODY_CONTENT_MAINTYPE_UNKOWN;
		parser->content_subtype = HTTP_BODY_CONTENT_SUBTYPE_UNKOWN;
		parser->content_charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
		parser->content_language = HTTP_BODY_CONTENT_LANGUAGE_UNKOWN;

		$$ = NULL;
	}
	| 
	{
		http_data_t *data = (http_data_t *)priv_data;
		http_parser_t *parser = &data->response_parser;

		parser->body_size = 0;
		$<body_info>$.content_type.maintype = parser->content_maintype;
		$<body_info>$.content_type.subtype = parser->content_subtype;
		$<body_info>$.content_encoding = parser->content_encoding;
		$<body_info>$.content_language = parser->content_language;
		$<body_info>$.content_charset = parser->content_charset;
		$<body_info>$.transfer_encoding = HTTP_BODY_TRANSFER_ENCODING_UNKOWN;

		if(parser->unzip_handle)
		{
			ey_zlib_destroy((ey_zlib_t)parser->unzip_handle);
			parser->unzip_handle = NULL;
		}
		
		assert(parser->html_work==NULL);
		if(priv_decoder->html_decoder && parser->content_maintype==HTTP_BODY_CONTENT_MAINTYPE_TEXT && parser->content_subtype==HTTP_BODY_CONTENT_SUBTYPE_HTML)
		{
			http_debug(debug_http_server_parser, "will create html work entry\n");
			parser->html_work = html_work_create2((html_handler_t)priv_decoder->html_decoder, 0, ((http_data_t*)priv_data)->engine_work);
			if(!parser->html_work)
			{
				http_debug(debug_http_server_parser, "create html work entry failed\n");
				YYABORT;
			}
		}

		if(parser->content_encoding==HTTP_BODY_CONTENT_ENCODING_GZIP ||
		   parser->content_encoding==HTTP_BODY_CONTENT_ENCODING_DEFLATE)
		{
			http_debug(debug_http_server_parser, "will create unzip entry\n");
			ey_zlib_format_t fmt = EY_ZLIB_FORMAT_GZIP_UNPACK;
			if(parser->content_encoding==HTTP_BODY_CONTENT_ENCODING_DEFLATE)
				fmt = EY_ZLIB_FORMAT_DEFLATE_UNPACK;
			parser->unzip_handle = ey_zlib_create(NULL, fmt, priv_data);
			if(!parser->unzip_handle)
			{
				http_debug(debug_http_server_parser, "create unzip entry failed\n");
				YYABORT;
			}
		}
	}
	response_normal_body 
	{
		http_data_t *data = (http_data_t *)priv_data;
		http_parser_t *parser = &data->response_parser;
		http_body_t *ret = http_server_alloc_body(priv_decoder);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc http normal body\n");
			YYABORT;
		}

		$<body_info>1.body_size = parser->body_size;
		ret->info = $<body_info>1;
		
		/*do html last slice check*/
		if(parser->html_work)
		{
			if(html_decode_data((html_work_t)parser->html_work, "", 0, 1))
			{
				http_debug(debug_http_server_parser, "find errors while parsing last html slice\n");
				html_work_destroy2((html_work_t)parser->html_work);
				parser->html_work = NULL;
			}
			else
			{
				http_debug(debug_http_server_parser, "parse last html slice successfully\n");
			}
		}
		ret->html_work = parser->html_work;
		parser->html_work = NULL;

		if(parser->unzip_handle)
		{
			ey_zlib_destroy((ey_zlib_t)parser->unzip_handle);
			parser->unzip_handle = NULL;
		}

		parser->content_encoding = HTTP_BODY_CONTENT_ENCODING_UNKOWN;
		parser->content_maintype = HTTP_BODY_CONTENT_MAINTYPE_UNKOWN;
		parser->content_subtype = HTTP_BODY_CONTENT_SUBTYPE_UNKOWN;
		parser->content_charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
		parser->content_language = HTTP_BODY_CONTENT_LANGUAGE_UNKOWN;
		parser->body_size = 0;

		STAILQ_CONCAT(&ret->normal_body, &$2);
		$$ = ret;
	}
	| 
	{
		http_data_t *data = (http_data_t *)priv_data;
		http_parser_t *parser = &data->response_parser;

		parser->body_size = 0;
		$<body_info>$.content_type.maintype = parser->content_maintype;
		$<body_info>$.content_type.subtype = parser->content_subtype;
		$<body_info>$.content_encoding = parser->content_encoding;
		$<body_info>$.content_language = parser->content_language;
		$<body_info>$.content_charset = parser->content_charset;
		$<body_info>$.transfer_encoding = HTTP_BODY_TRANSFER_ENCODING_CHUNKED;

		if(parser->unzip_handle)
		{
			ey_zlib_destroy((ey_zlib_t)parser->unzip_handle);
			parser->unzip_handle = NULL;
		}

		assert(parser->html_work==NULL);
		if(priv_decoder->html_decoder && parser->content_maintype==HTTP_BODY_CONTENT_MAINTYPE_TEXT && parser->content_subtype==HTTP_BODY_CONTENT_SUBTYPE_HTML)
		{
			http_debug(debug_http_server_parser, "will create html work entry\n");
			parser->html_work = html_work_create2((html_handler_t)priv_decoder->html_decoder, 0, ((http_data_t*)priv_data)->engine_work);
			if(!parser->html_work)
			{
				http_debug(debug_http_server_parser, "create html work entry failed\n");
				YYABORT;
			}
		}

		if(parser->content_encoding==HTTP_BODY_CONTENT_ENCODING_GZIP ||
		   parser->content_encoding==HTTP_BODY_CONTENT_ENCODING_DEFLATE)
		{
			http_debug(debug_http_server_parser, "will create unzip entry\n");
			ey_zlib_format_t fmt = EY_ZLIB_FORMAT_GZIP_UNPACK;
			if(parser->content_encoding==HTTP_BODY_CONTENT_ENCODING_DEFLATE)
				fmt = EY_ZLIB_FORMAT_DEFLATE_UNPACK;
			parser->unzip_handle = ey_zlib_create(NULL, fmt, priv_data);
			if(!parser->unzip_handle)
			{
				http_debug(debug_http_server_parser, "create unzip entry failed\n");
				YYABORT;
			}
		}
	}
	response_chunk_body
	{
		http_data_t *data = (http_data_t *)priv_data;
		http_parser_t *parser = &data->response_parser;
		http_body_t *ret = http_server_alloc_body(priv_decoder);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc http chunk body\n");
			YYABORT;
		}

		$<body_info>1.body_size = parser->body_size;
		ret->info = $<body_info>1;

		/*do html last slice check*/
		if(parser->html_work)
		{
			if(html_decode_data((html_work_t)parser->html_work, "", 0, 1))
			{
				http_debug(debug_http_server_parser, "find errors while parsing last html slice\n");
				html_work_destroy2((html_work_t)parser->html_work);
				parser->html_work = NULL;
			}
			else
			{
				http_debug(debug_http_server_parser, "parse last html slice successfully\n");
			}
		}
		ret->html_work = parser->html_work;
		parser->html_work = NULL;

		if(parser->unzip_handle)
		{
			ey_zlib_destroy((ey_zlib_t)parser->unzip_handle);
			parser->unzip_handle = NULL;
		}

		parser->content_encoding = HTTP_BODY_CONTENT_ENCODING_UNKOWN;
		parser->content_maintype = HTTP_BODY_CONTENT_MAINTYPE_UNKOWN;
		parser->content_subtype = HTTP_BODY_CONTENT_SUBTYPE_UNKOWN;
		parser->content_charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
		parser->content_language = HTTP_BODY_CONTENT_LANGUAGE_UNKOWN;
		parser->body_size = 0;

		STAILQ_CONCAT(&ret->chunk_body.chunk_list, &$2.chunk_list);
		STAILQ_CONCAT(&ret->chunk_body.chunk_tailer, &$2.chunk_tailer);
		$$ = ret;
	}
	;

response_normal_body:
	response_body_part
	{
		http_string_list_part_t *ret = http_server_alloc_string_list_part(priv_decoder, &$1);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc normal body data part\n");
			YYABORT;
		}
		STAILQ_INIT(&$$);
		STAILQ_INSERT_TAIL(&$$, ret, next);
	}
	| response_normal_body response_body_part
	{
		http_string_list_part_t *ret = http_server_alloc_string_list_part(priv_decoder, &$2);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc normal body data part\n");
			YYABORT;
		}
		$$ = $1;
		STAILQ_INSERT_TAIL(&$$, ret, next);
	}
	;

response_chunk_body:
	response_chunk_list response_chunk_tailer_list
	{
		STAILQ_INIT(&$$.chunk_list);
		STAILQ_INIT(&$$.chunk_tailer);

		STAILQ_CONCAT(&$$.chunk_list, &$1);
		STAILQ_CONCAT(&$$.chunk_tailer, &$2);
	}
	;

response_chunk_list:
	response_chunk
	{
		http_chunk_body_part_t *saved_chunk = $1;
		STAILQ_INIT(&$$);
		STAILQ_INSERT_TAIL(&$$, saved_chunk, next);
	}
	| response_chunk_list response_chunk
	{
		$$ = $1;
		STAILQ_INSERT_TAIL(&$$, $2, next);
	}
	;

response_chunk:
	response_chunk_header response_chunk_data
	{
		http_chunk_body_part_t *ret = http_server_alloc_chunk_body_part(priv_decoder);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc chunk part\n");
			YYABORT;
		}
		ret->chunk_header = $1;
		STAILQ_CONCAT(&ret->chunk_value, &$2);
		$$ = ret;
	}
	;

response_chunk_header:
	TOKEN_SERVER_BODY_CHUNK_HEADER
	{
		if(http_client_parse_chunk_header(priv_decoder, &$1, &$$))
		{
			http_debug(debug_http_server_parser, "failed to chunk header\n");
			YYABORT;
		}
	}
	;

response_chunk_data:
	response_empty
	{
		STAILQ_INIT(&$$);
	}
	| response_chunk_data response_body_part
	{
		http_string_list_part_t *ret = http_server_alloc_string_list_part(priv_decoder, &$2);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc chunk data part\n");
			YYABORT;
		}
		$$ = $1;
		STAILQ_INSERT_TAIL(&$$, ret, next);
	}
	;

response_chunk_tailer_list:
	response_empty
	{
		STAILQ_INIT(&$$);
	}
	| response_chunk_tailer_list TOKEN_SERVER_BODY_CHUNK_TAILER
	{
		http_string_t dup_string = {NULL, 0};
		if(!http_server_alloc_string(priv_decoder, $2.buf, $2.len, &dup_string))
		{
			http_debug(debug_http_server_parser, "alloc chunk tailer value failed\n");
			YYABORT;
		}

		http_string_list_part_t *ret = http_server_alloc_string_list_part(priv_decoder, &dup_string);
		if(!ret)
		{
			http_debug(debug_http_server_parser, "failed to alloc chunk tailer part\n");
			http_server_free_string(priv_decoder, &dup_string);
			YYABORT;
		}
		$$ = $1;
		STAILQ_INSERT_TAIL(&$$, ret, next);
	}
	;

response_body_part:
	TOKEN_SERVER_BODY_PART
	{
		http_data_t *data = (http_data_t *)priv_data;
		http_parser_t *parser = &data->response_parser;
		http_string_t dup_string = {NULL, 0};

		if(!parser->unzip_handle)
		{
			if(!http_server_alloc_string(priv_decoder, $1.buf, $1.len, &dup_string))
			{
				http_debug(debug_http_server_parser, "failed to duplicate body part value\n");
				YYABORT;
			}
		}
		else
		{
			if(http_server_unzip_string(data, &$1, &dup_string))
			{
				http_debug(debug_http_server_parser, "failed to unzip body part value\n");
				YYABORT;
			}
		}
		
		if(parser->html_work)
		{
			if(html_decode_data((html_work_t)parser->html_work, dup_string.buf, dup_string.len, 0))
			{
				http_debug(debug_http_server_parser, "find errors while parsing html slice\n");
				html_work_destroy2((html_work_t)parser->html_work);
				parser->html_work = NULL;
			}
			else
			{
				http_debug(debug_http_server_parser, "parse last html slice successfully\n");
			}
		}

		parser->body_size += dup_string.len;
		$$ = dup_string;
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
		if(http_server_lex_body_mode(lexier))
			token = http_server_body_lex(&value, lexier);
		else
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
