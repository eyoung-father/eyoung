#ifndef HTTP_TYPE_H
#define HTTP_TYPE_H 1

#include "ey_queue.h"
#include "ey_string.h"
#include "http_decode.h"
#include "libengine.h"

/*common info*/

typedef ey_string_t http_string_t;
typedef struct http_string_list_part
{
	http_string_t string;
	STAILQ_ENTRY(http_string_list_part) next;
}http_string_list_part_t;
typedef STAILQ_HEAD(http_string_list, http_string_list_part) http_string_list_t;

/*
 * HTTP VERSION
 * */
typedef enum http_version
{
	HTTP_VERSION_09,
	HTTP_VERSION_10,
	HTTP_VERSION_11,

	HTTP_VERSION_UNKOWN
}http_version_t;

static inline const char* http_version_name(http_version_t version)
{
	switch(version)
	{
		case HTTP_VERSION_09:
			return "HTTP/0.9";
		case HTTP_VERSION_10:
			return "HTTP/1.0";
		case HTTP_VERSION_11:
			return "HTTP/1.1";
		case HTTP_VERSION_UNKOWN:
		default:
			return "(UNKOWN)";
	}
}

/*
 * BODY CONTENT MAIN-TYPE
 * */
typedef enum http_body_content_maintype
{
	HTTP_BODY_CONTENT_MAINTYPE_TEXT,
	HTTP_BODY_CONTENT_MAINTYPE_IMAGE,
	HTTP_BODY_CONTENT_MAINTYPE_AUDIO,
	HTTP_BODY_CONTENT_MAINTYPE_VIDEO,
	HTTP_BODY_CONTENT_MAINTYPE_APPLICATION,
	HTTP_BODY_CONTENT_MAINTYPE_DRAWING,
	HTTP_BODY_CONTENT_MAINTYPE_MODEL,
	HTTP_BODY_CONTENT_MAINTYPE_MESSAGE,
	HTTP_BODY_CONTENT_MAINTYPE_MULTIPART,

	HTTP_BODY_CONTENT_MAINTYPE_UNKOWN
}http_body_content_maintype_t;

static inline const char *http_body_content_maintype_name(http_body_content_maintype_t type)
{
	switch(type)
	{
		case HTTP_BODY_CONTENT_MAINTYPE_TEXT:
			return "text";
		case HTTP_BODY_CONTENT_MAINTYPE_IMAGE:
			return "image";
		case HTTP_BODY_CONTENT_MAINTYPE_AUDIO:
			return "audio";
		case HTTP_BODY_CONTENT_MAINTYPE_VIDEO:
			return "video";
		case HTTP_BODY_CONTENT_MAINTYPE_APPLICATION:
			return "application";
		case HTTP_BODY_CONTENT_MAINTYPE_DRAWING:
			return "drawing";
		case HTTP_BODY_CONTENT_MAINTYPE_MODEL:
			return "model";
		case HTTP_BODY_CONTENT_MAINTYPE_MESSAGE:
			return "message";
		case HTTP_BODY_CONTENT_MAINTYPE_MULTIPART:
			return "multipart";
		case HTTP_BODY_CONTENT_MAINTYPE_UNKOWN:
		default:
			return "UNKOWN";
	}
}

/*
 * BODY CONTENT SUB-TYPE
 * */
typedef enum http_body_content_subtype
{
	HTTP_BODY_CONTENT_SUBTYPE_UNKOWN
}http_body_content_subtype_t;

static inline const char *http_body_content_subtype_name(http_body_content_subtype_t type)
{
	switch(type)
	{
		case HTTP_BODY_CONTENT_SUBTYPE_UNKOWN:
		default:
			return "UNKOWN";
	}
}

/*
 * BODY CONTENT-TYPE
 * */
typedef struct http_body_content_type
{
	http_body_content_maintype_t maintype;
	http_body_content_subtype_t subtype;
}http_body_content_type_t;

/*
 * BODY CONTENT-ENCODING
 * */
typedef enum http_body_content_encoding
{
	HTTP_BODY_CONTENT_ENCODING_GZIP,
	HTTP_BODY_CONTENT_ENCODING_COMPRESS,
	HTTP_BODY_CONTENT_ENCODING_DEFLATE,
	HTTP_BODY_CONTENT_ENCODING_IDENTITY,

	HTTP_BODY_CONTENT_ENCODING_UNKOWN
}http_body_content_encoding_t;

static inline const char *http_body_content_encoding_name(http_body_content_encoding_t type)
{
	switch(type)
	{
		case HTTP_BODY_CONTENT_ENCODING_GZIP:
			return "gzip";
		case HTTP_BODY_CONTENT_ENCODING_COMPRESS:
			return "compress";
		case HTTP_BODY_CONTENT_ENCODING_DEFLATE:
			return "deflate";
		case HTTP_BODY_CONTENT_ENCODING_IDENTITY:
			return "identity";
		case HTTP_BODY_CONTENT_ENCODING_UNKOWN:
		default:
			return "UNKOWN";
	}
}

/*
 * BODY TRANSFER-ENCODING
 * */
typedef enum http_body_transfer_encoding
{
	HTTP_BODY_TRANSFER_ENCODING_CHUNKED,

	HTTP_BODY_TRANSFER_ENCODING_UNKOWN
}http_body_transfer_encoding_t;

static inline const char *http_body_transfer_encoding_name(http_body_transfer_encoding_t type)
{
	switch(type)
	{
		case HTTP_BODY_TRANSFER_ENCODING_CHUNKED:
			return "chunked";
		case HTTP_BODY_TRANSFER_ENCODING_UNKOWN:
		default:
			return "UNKOWN";
	}
}

/*
 * BODY CONTENT-LANGUAGE
 * */
typedef enum http_body_content_language
{
	HTTP_BODY_CONTENT_LANGUAGE_UNKOWN
}http_body_content_language_t;

static inline const char *http_body_content_language_name(http_body_content_language_t type)
{
	switch(type)
	{
		case HTTP_BODY_CONTENT_LANGUAGE_UNKOWN:
		default:
			return "UNKOWN";
	}
}

/*
 * BODY CONTENT-CHARSET
 * */
typedef enum http_body_content_charset
{
	HTTP_BODY_CONTENT_CHARSET_UNKOWN
}http_body_content_charset_t;

static inline const char *http_body_content_charset_name(http_body_content_charset_t type)
{
	switch(type)
	{
		case HTTP_BODY_CONTENT_CHARSET_UNKOWN:
		default:
			return "UNKOWN";
	}
}

typedef struct http_body_info
{
	http_body_content_type_t content_type;
	http_body_content_encoding_t content_encoding;
	http_body_transfer_encoding_t transfer_encoding;
	http_body_content_language_t content_language;
	http_body_content_charset_t content_charset;
	size_t body_size;
}http_body_info_t;

/*
 * CHUNKED BODY
 * */
typedef struct http_chunk_body_header
{
	size_t chunk_size;
	http_string_t chunk_extension;
}http_chunk_body_header_t;

typedef struct http_chunk_body_part
{
	http_chunk_body_header_t chunk_header;
	http_string_list_t chunk_value;
	STAILQ_ENTRY(http_chunk_body_part) next;
}http_chunk_body_part_t;
typedef STAILQ_HEAD(http_chunk_body_list, http_chunk_body_part) http_chunk_body_list_t;

typedef struct http_chunk_body
{
	http_chunk_body_list_t chunk_list;
	http_string_list_t chunk_tailer;
}http_chunk_body_t;

typedef struct http_body
{
	http_body_info_t info;
	http_string_list_t normal_body;
	http_chunk_body_t chunk_body;
}http_body_t;

/*
 * Client Message Type
 */
typedef enum http_request_method
{
	HTTP_REQUEST_METHOD_METHOD_GET = 1,
	HTTP_REQUEST_METHOD_METHOD_POST,
	HTTP_REQUEST_METHOD_METHOD_HEAD,
	HTTP_REQUEST_METHOD_METHOD_OPTIONS,
	HTTP_REQUEST_METHOD_METHOD_PUT,
	HTTP_REQUEST_METHOD_METHOD_DELETE,
	HTTP_REQUEST_METHOD_METHOD_TRACE,
	HTTP_REQUEST_METHOD_METHOD_CONNECT,
	HTTP_REQUEST_METHOD_METHOD_PATCH,
	HTTP_REQUEST_METHOD_METHOD_LINK,
	HTTP_REQUEST_METHOD_METHOD_UNLINK,
	HTTP_REQUEST_METHOD_METHOD_PROPFIND,
	HTTP_REQUEST_METHOD_METHOD_PROPPATCH,
	HTTP_REQUEST_METHOD_METHOD_MKCOL,
	HTTP_REQUEST_METHOD_METHOD_COPY,
	HTTP_REQUEST_METHOD_METHOD_MOVE,
	HTTP_REQUEST_METHOD_METHOD_LOCK,
	HTTP_REQUEST_METHOD_METHOD_UNLOCK,
	HTTP_REQUEST_METHOD_METHOD_CHECKOUT,
	HTTP_REQUEST_METHOD_METHOD_REPORT,
	HTTP_REQUEST_METHOD_METHOD_VERSION_CONTROL,
	HTTP_REQUEST_METHOD_METHOD_CHECKIN,
	HTTP_REQUEST_METHOD_METHOD_UNCHECKOUT,
	HTTP_REQUEST_METHOD_METHOD_MKWORKSPACE,
	HTTP_REQUEST_METHOD_METHOD_UPDATE,
	HTTP_REQUEST_METHOD_METHOD_LABEL,
	HTTP_REQUEST_METHOD_METHOD_MERGE,
	HTTP_REQUEST_METHOD_METHOD_MKACTIVITY,
	HTTP_REQUEST_METHOD_METHOD_ORDERPATCH,
	HTTP_REQUEST_METHOD_METHOD_BASELINE_CONTROL,

	HTTP_REQUEST_METHOD_METHOD_UNKOWN
}http_request_method_t;

static inline const char *http_request_method_name(http_request_method_t method)
{
	switch(method)
	{
		case HTTP_REQUEST_METHOD_METHOD_GET:
			return "GET";
		case HTTP_REQUEST_METHOD_METHOD_POST:
			return "POST";
		case HTTP_REQUEST_METHOD_METHOD_HEAD:
			return "HEAD";
		case HTTP_REQUEST_METHOD_METHOD_OPTIONS:
			return "OPTIONS";
		case HTTP_REQUEST_METHOD_METHOD_PUT:
			return "PUT";
		case HTTP_REQUEST_METHOD_METHOD_DELETE:
			return "DELETE";
		case HTTP_REQUEST_METHOD_METHOD_TRACE:
			return "TRACE";
		case HTTP_REQUEST_METHOD_METHOD_CONNECT:
			return "CONNECT";
		case HTTP_REQUEST_METHOD_METHOD_PATCH:
			return "PATCH";
		case HTTP_REQUEST_METHOD_METHOD_LINK:
			return "LINK";
		case HTTP_REQUEST_METHOD_METHOD_UNLINK:
			return "UNLINK";
		case HTTP_REQUEST_METHOD_METHOD_PROPFIND:
			return "PROPFIND";
		case HTTP_REQUEST_METHOD_METHOD_PROPPATCH:
			return "PROPPATCH";
		case HTTP_REQUEST_METHOD_METHOD_MKCOL:
			return "MKCOL";
		case HTTP_REQUEST_METHOD_METHOD_COPY:
			return "COPY";
		case HTTP_REQUEST_METHOD_METHOD_MOVE:
			return "MOVE";
		case HTTP_REQUEST_METHOD_METHOD_LOCK:
			return "LOCK";
		case HTTP_REQUEST_METHOD_METHOD_UNLOCK:
			return "UNLOCK";
		case HTTP_REQUEST_METHOD_METHOD_CHECKOUT:
			return "CHECKOUT";
		case HTTP_REQUEST_METHOD_METHOD_REPORT:
			return "REPORT";
		case HTTP_REQUEST_METHOD_METHOD_VERSION_CONTROL:
			return "VERSION-CONTROL";
		case HTTP_REQUEST_METHOD_METHOD_CHECKIN:
			return "CHECKIN";
		case HTTP_REQUEST_METHOD_METHOD_UNCHECKOUT:
			return "UNCHECKOUT";
		case HTTP_REQUEST_METHOD_METHOD_MKWORKSPACE:
			return "MKWORKSPACE";
		case HTTP_REQUEST_METHOD_METHOD_UPDATE:
			return "UPDATE";
		case HTTP_REQUEST_METHOD_METHOD_LABEL:
			return "LABEL";
		case HTTP_REQUEST_METHOD_METHOD_MERGE:
			return "MERGE";
		case HTTP_REQUEST_METHOD_METHOD_MKACTIVITY:
			return "MKACTIVITY";
		case HTTP_REQUEST_METHOD_METHOD_ORDERPATCH:
			return "ORDERPATCH";
		case HTTP_REQUEST_METHOD_METHOD_BASELINE_CONTROL:
			return "BASELINE-CONTROL";
		case HTTP_REQUEST_METHOD_METHOD_UNKOWN:
		default:
			return "(UNKOWN)";
	}
}

typedef struct http_request_first_line
{
	http_request_method_t method;
	http_version_t version;
	http_string_t uri;
}http_request_first_line_t;

typedef enum http_request_header_type
{
	HTTP_REQUEST_HEADER_HOST,
	HTTP_REQUEST_HEADER_CACHE_CONTROL,
	HTTP_REQUEST_HEADER_CONNECTION,
	HTTP_REQUEST_HEADER_DATE,
	HTTP_REQUEST_HEADER_PRAGMA,
	HTTP_REQUEST_HEADER_TRAILER, 
	HTTP_REQUEST_HEADER_TRANSFER_ENCODING,
	HTTP_REQUEST_HEADER_UPGRADE,
	HTTP_REQUEST_HEADER_VIA,
	HTTP_REQUEST_HEADER_WARNING,
	HTTP_REQUEST_HEADER_MIME_VERSION,
	HTTP_REQUEST_HEADER_ALLOW,
	HTTP_REQUEST_HEADER_CONTENT_ENCODING,
	HTTP_REQUEST_HEADER_CONTENT_LANGUAGE,
	HTTP_REQUEST_HEADER_CONTENT_LENGTH,
	HTTP_REQUEST_HEADER_CONTENT_LOCATION,
	HTTP_REQUEST_HEADER_CONTENT_MD5,
	HTTP_REQUEST_HEADER_CONTENT_RANGE,
	HTTP_REQUEST_HEADER_CONTENT_TYPE,
	HTTP_REQUEST_HEADER_ETAG,
	HTTP_REQUEST_HEADER_EXPIRES,
	HTTP_REQUEST_HEADER_LAST_MODIFIED,
	HTTP_REQUEST_HEADER_CONTENT_BASE,
	HTTP_REQUEST_HEADER_CONTENT_VERSION,
	HTTP_REQUEST_HEADER_DERIVED_FROM,
	HTTP_REQUEST_HEADER_LINK,
	HTTP_REQUEST_HEADER_KEEP_ALIVE,
	HTTP_REQUEST_HEADER_URI,
	HTTP_REQUEST_HEADER_ACCEPT_CHARSET,
	HTTP_REQUEST_HEADER_ACCEPT_ENCODING,
	HTTP_REQUEST_HEADER_ACCEPT_LANGUAGE,
	HTTP_REQUEST_HEADER_ACCEPT,
	HTTP_REQUEST_HEADER_AUTHORIZATION,
	HTTP_REQUEST_HEADER_EXCEPT,
	HTTP_REQUEST_HEADER_FROM,
	HTTP_REQUEST_HEADER_IF_MATCH,
	HTTP_REQUEST_HEADER_IF_MODIFIED_SINCE,
	HTTP_REQUEST_HEADER_IF_NONE_MATCH,
	HTTP_REQUEST_HEADER_IF_RANGE,
	HTTP_REQUEST_HEADER_IF_UNMODIFIED_SINCE,
	HTTP_REQUEST_HEADER_MAX_FORWARDS,
	HTTP_REQUEST_HEADER_RANGE,
	HTTP_REQUEST_HEADER_REFERER,
	HTTP_REQUEST_HEADER_TE,
	HTTP_REQUEST_HEADER_USER_AGENT,
	HTTP_REQUEST_HEADER_COOKIE2,
	HTTP_REQUEST_HEADER_COOKIE,
	HTTP_REQUEST_HEADER_UA_PIXELS,
	HTTP_REQUEST_HEADER_UA_COLOR,
	HTTP_REQUEST_HEADER_UA_OS,
	HTTP_REQUEST_HEADER_UA_CPU,
	HTTP_REQUEST_HEADER_X_FLASH_VERSION,
	HTTP_REQUEST_HEADER_PROXY_AUTHORIZATION,

	HTTP_REQUEST_HEADER_UNKOWN
}http_request_header_type_t;

static inline const char* http_request_header_name(http_request_header_type_t type)
{
	switch(type)
	{
		case HTTP_REQUEST_HEADER_HOST:
			return "HOST";
		case HTTP_REQUEST_HEADER_CACHE_CONTROL:
			return "CACHE-CONTROL";
		case HTTP_REQUEST_HEADER_CONNECTION:
			return "CONNECTION";
		case HTTP_REQUEST_HEADER_DATE:
			return "DATE";
		case HTTP_REQUEST_HEADER_PRAGMA:
			return "PRAGMA";
		case HTTP_REQUEST_HEADER_TRAILER:
			return "TRAILER";
		case HTTP_REQUEST_HEADER_TRANSFER_ENCODING:
			return "TRANSFER-ENCODING";
		case HTTP_REQUEST_HEADER_UPGRADE:
			return "UPGRADE";
		case HTTP_REQUEST_HEADER_VIA:
			return "VIA";
		case HTTP_REQUEST_HEADER_WARNING:
			return "WARNING";
		case HTTP_REQUEST_HEADER_MIME_VERSION:
			return "MIME-VERSION";
		case HTTP_REQUEST_HEADER_ALLOW:
			return "ALLOW";
		case HTTP_REQUEST_HEADER_CONTENT_ENCODING:
			return "CONTENT-ENCODING";
		case HTTP_REQUEST_HEADER_CONTENT_LANGUAGE:
			return "CONTENT-LANGUAGE";
		case HTTP_REQUEST_HEADER_CONTENT_LENGTH:
			return "CONTENT-LENGTH";
		case HTTP_REQUEST_HEADER_CONTENT_LOCATION:
			return "CONTENT-LOCATION";
		case HTTP_REQUEST_HEADER_CONTENT_MD5:
			return "CONTENT-MD5";
		case HTTP_REQUEST_HEADER_CONTENT_RANGE:
			return "CONTENT-RANGE";
		case HTTP_REQUEST_HEADER_CONTENT_TYPE:
			return "CONTENT-TYPE";
		case HTTP_REQUEST_HEADER_ETAG:
			return "ETAG";
		case HTTP_REQUEST_HEADER_EXPIRES:
			return "EXPIRES";
		case HTTP_REQUEST_HEADER_LAST_MODIFIED:
			return "LAST-MODIFIED";
		case HTTP_REQUEST_HEADER_CONTENT_BASE:
			return "CONTENT-BASE";
		case HTTP_REQUEST_HEADER_CONTENT_VERSION:
			return "CONTENT-VERSION";
		case HTTP_REQUEST_HEADER_DERIVED_FROM:
			return "DERIVED-FROM";
		case HTTP_REQUEST_HEADER_LINK:
			return "LINK";
		case HTTP_REQUEST_HEADER_KEEP_ALIVE:
			return "KEEP-ALIVE";
		case HTTP_REQUEST_HEADER_URI:
			return "URI";
		case HTTP_REQUEST_HEADER_ACCEPT_CHARSET:
			return "ACCEPT-CHARSET";
		case HTTP_REQUEST_HEADER_ACCEPT_ENCODING:
			return "ACCEPT-ENCODING";
		case HTTP_REQUEST_HEADER_ACCEPT_LANGUAGE:
			return "ACCEPT-LANGUAGE";
		case HTTP_REQUEST_HEADER_ACCEPT:
			return "ACCEPT";
		case HTTP_REQUEST_HEADER_AUTHORIZATION:
			return "AUTHORIZATION";
		case HTTP_REQUEST_HEADER_EXCEPT:
			return "EXCEPT";
		case HTTP_REQUEST_HEADER_FROM:
			return "FROM";
		case HTTP_REQUEST_HEADER_IF_MATCH:
			return "IF-MATCH";
		case HTTP_REQUEST_HEADER_IF_MODIFIED_SINCE:
			return "IF-MODIFIED-SINCE";
		case HTTP_REQUEST_HEADER_IF_NONE_MATCH:
			return "IF-NONE-MATCH";
		case HTTP_REQUEST_HEADER_IF_RANGE:
			return "IF-RANGE";
		case HTTP_REQUEST_HEADER_IF_UNMODIFIED_SINCE:
			return "IF-UNMODIFIED-SINCE";
		case HTTP_REQUEST_HEADER_MAX_FORWARDS:
			return "MAX-FORWARDS";
		case HTTP_REQUEST_HEADER_RANGE:
			return "RANGE";
		case HTTP_REQUEST_HEADER_REFERER:
			return "REFERER";
		case HTTP_REQUEST_HEADER_TE:
			return "TE";
		case HTTP_REQUEST_HEADER_USER_AGENT:
			return "USER-AGENT";
		case HTTP_REQUEST_HEADER_COOKIE2:
			return "COOKIE2";
		case HTTP_REQUEST_HEADER_COOKIE:
			return "COOKIE";
		case HTTP_REQUEST_HEADER_UA_PIXELS:
			return "UA-PIXELS";
		case HTTP_REQUEST_HEADER_UA_COLOR:
			return "UA-COLOR";
		case HTTP_REQUEST_HEADER_UA_OS:
			return "UA-OS";
		case HTTP_REQUEST_HEADER_UA_CPU:
			return "UA-CPU";
		case HTTP_REQUEST_HEADER_X_FLASH_VERSION:
			return "X-FLASH-VERSION";
		case HTTP_REQUEST_HEADER_PROXY_AUTHORIZATION:
			return "PROXY-AUTHORIZATION";
		case HTTP_REQUEST_HEADER_UNKOWN:
		default:
			return "(UNKOWN)";
	}
}

typedef struct http_request_header
{
	http_request_header_type_t type;
	http_string_t value;
	STAILQ_ENTRY(http_request_header) next;
}http_request_header_t;
typedef STAILQ_HEAD(http_request_header_list, http_request_header) http_request_header_list_t;

typedef struct http_request
{
	http_request_first_line_t *first_line;
	http_request_header_list_t header_list;
	http_body_t *body;

	STAILQ_ENTRY(http_request) next;
}http_request_t;

typedef STAILQ_HEAD(http_request_list, http_request) http_request_list_t;

/*
 * Server Message Type
 */
typedef int http_response_code_t;

typedef struct http_response_first_line
{
	http_version_t version;
	http_response_code_t code;
	http_string_t message;
}http_response_first_line_t;

typedef enum http_response_header_type
{
	HTTP_RESPONSE_HEADER_CACHE_CONTROL,
	HTTP_RESPONSE_HEADER_CONNECTION,
	HTTP_RESPONSE_HEADER_DATE,
	HTTP_RESPONSE_HEADER_PRAGMA,
	HTTP_RESPONSE_HEADER_TRAILER,
	HTTP_RESPONSE_HEADER_TRANSFER_ENCODING,
	HTTP_RESPONSE_HEADER_UPGRADE,
	HTTP_RESPONSE_HEADER_VIA,
	HTTP_RESPONSE_HEADER_WARNING,
	HTTP_RESPONSE_HEADER_MIME_VERSION,
	HTTP_RESPONSE_HEADER_ALLOW,
	HTTP_RESPONSE_HEADER_CONTENT_ENCODING,
	HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE,
	HTTP_RESPONSE_HEADER_CONTENT_LENGTH,
	HTTP_RESPONSE_HEADER_CONTENT_LOCATION,
	HTTP_RESPONSE_HEADER_CONTENT_MD5,
	HTTP_RESPONSE_HEADER_CONTENT_RANGE,
	HTTP_RESPONSE_HEADER_CONTENT_TYPE,
	HTTP_RESPONSE_HEADER_ETAG,
	HTTP_RESPONSE_HEADER_EXPIRES,
	HTTP_RESPONSE_HEADER_LAST_MODIFIED,
	HTTP_RESPONSE_HEADER_CONTENT_BASE,
	HTTP_RESPONSE_HEADER_CONTENT_VERSION,
	HTTP_RESPONSE_HEADER_DERIVED_FROM,
	HTTP_RESPONSE_HEADER_LINK,
	HTTP_RESPONSE_HEADER_KEEP_ALIVE,
	HTTP_RESPONSE_HEADER_URI,
	HTTP_RESPONSE_HEADER_ACCEPT_RANGES,
	HTTP_RESPONSE_HEADER_AGE,
	HTTP_RESPONSE_HEADER_LOCATION,
	HTTP_RESPONSE_HEADER_RETRY_AFTER,
	HTTP_RESPONSE_HEADER_SERVER,
	HTTP_RESPONSE_HEADER_VARY,
	HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE,
	HTTP_RESPONSE_HEADER_SET_COOKIE2,
	HTTP_RESPONSE_HEADER_SET_COOKIE,
	HTTP_RESPONSE_HEADER_X_POWERED_BY,
	HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE,
	HTTP_RESPONSE_HEADER_UNKOWN
}http_response_header_type_t;

static inline const char* http_response_header_name(http_response_header_type_t type)
{
	switch(type)
	{
		case HTTP_RESPONSE_HEADER_CACHE_CONTROL:
			return "CACHE-CONTROL";
		case HTTP_RESPONSE_HEADER_CONNECTION:
			return "CONNECTION";
		case HTTP_RESPONSE_HEADER_DATE:
			return "DATE";
		case HTTP_RESPONSE_HEADER_PRAGMA:
			return "PRAGMA";
		case HTTP_RESPONSE_HEADER_TRAILER:
			return "TRAILER";
		case HTTP_RESPONSE_HEADER_TRANSFER_ENCODING:
			return "TRANSFER-ENCODING";
		case HTTP_RESPONSE_HEADER_UPGRADE:
			return "UPGRADE";
		case HTTP_RESPONSE_HEADER_VIA:
			return "VIA";
		case HTTP_RESPONSE_HEADER_WARNING:
			return "WARNING";
		case HTTP_RESPONSE_HEADER_MIME_VERSION:
			return "MIME-VERSION";
		case HTTP_RESPONSE_HEADER_ALLOW:
			return "ALLOW";
		case HTTP_RESPONSE_HEADER_CONTENT_ENCODING:
			return "CONTENT-ENCODING";
		case HTTP_RESPONSE_HEADER_CONTENT_LANGUAGE:
			return "CONTENT-LANGUAGE";
		case HTTP_RESPONSE_HEADER_CONTENT_LENGTH:
			return "CONTENT-LENGTH";
		case HTTP_RESPONSE_HEADER_CONTENT_LOCATION:
			return "CONTENT-LOCATION";
		case HTTP_RESPONSE_HEADER_CONTENT_MD5:
			return "CONTENT-MD5";
		case HTTP_RESPONSE_HEADER_CONTENT_RANGE:
			return "CONTENT-RANGE";
		case HTTP_RESPONSE_HEADER_CONTENT_TYPE:
			return "CONTENT-TYPE";
		case HTTP_RESPONSE_HEADER_ETAG:
			return "ETAG";
		case HTTP_RESPONSE_HEADER_EXPIRES:
			return "EXPIRES";
		case HTTP_RESPONSE_HEADER_LAST_MODIFIED:
			return "LAST-MODIFIED";
		case HTTP_RESPONSE_HEADER_CONTENT_BASE:
			return "CONTENT-BASE";
		case HTTP_RESPONSE_HEADER_CONTENT_VERSION:
			return "CONTENT-VERSION";
		case HTTP_RESPONSE_HEADER_DERIVED_FROM:
			return "DERIVED-FROM";
		case HTTP_RESPONSE_HEADER_LINK:
			return "LINK";
		case HTTP_RESPONSE_HEADER_KEEP_ALIVE:
			return "KEEP-ALIVE";
		case HTTP_RESPONSE_HEADER_URI:
			return "URI";
		case HTTP_RESPONSE_HEADER_ACCEPT_RANGES:
			return "ACCEPT-RANGES";
		case HTTP_RESPONSE_HEADER_AGE:
			return "AGE";
		case HTTP_RESPONSE_HEADER_LOCATION:
			return "LOCATION";
		case HTTP_RESPONSE_HEADER_RETRY_AFTER:
			return "RETRY-AFTER";
		case HTTP_RESPONSE_HEADER_SERVER:
			return "SERVER";
		case HTTP_RESPONSE_HEADER_VARY:
			return "VARY";
		case HTTP_RESPONSE_HEADER_WWW_AUTHENTICATE:
			return "WWW-AUTHENTICATE";
		case HTTP_RESPONSE_HEADER_SET_COOKIE2:
			return "SET-COOKIE2";
		case HTTP_RESPONSE_HEADER_SET_COOKIE:
			return "SET-COOKIE";
		case HTTP_RESPONSE_HEADER_X_POWERED_BY:
			return "X-POWERED-BY";
		case HTTP_RESPONSE_HEADER_PROXY_AUTHENTICATE:
			return "PROXY-AUTHENTICATE";
		case HTTP_RESPONSE_HEADER_UNKOWN:
		default:
			return "(UNKOWN)";
	}
}

typedef struct http_response_header
{
	http_response_header_type_t type;
	http_string_t value;

	STAILQ_ENTRY(http_response_header) next;
}http_response_header_t;
typedef STAILQ_HEAD(http_resposne_header_list, http_response_header) http_response_header_list_t;

typedef struct http_response
{
	http_response_first_line_t *first_line;
	http_response_header_list_t header_list;
	http_body_t *body;

	STAILQ_ENTRY(http_response) next;
}http_response_t;

typedef STAILQ_HEAD(http_response_list, http_response) http_response_list_t;

/*
 * http session command
 */
typedef struct http_transaction
{
	http_request_t *request;
	http_response_t *response;

	STAILQ_ENTRY(http_transaction) next;
}http_transaction_t;

typedef STAILQ_HEAD(http_transaction_list, http_transaction) http_transaction_list_t;

/*
 * http parser
 */
typedef struct http_parser
{
	void *parser;
	void *lexier;

	char *saved;
	int saved_len;
	char last_frag;
	char greedy;
	char chunked;
	size_t length;
	http_body_content_encoding_t content_encoding;
	http_body_content_maintype_t content_maintype;
	http_body_content_subtype_t content_subtype;
	http_body_content_charset_t content_charset;
	http_body_content_language_t content_language;
}http_parser_t;

/*
 * http private data
 */
typedef struct http_data
{
	http_handler_t decoder;
	engine_work_t *engine_work;

	/*client ==> server*/
	http_parser_t request_parser;
	http_request_list_t request_list;

	/*server ==> client*/
	http_parser_t response_parser;
	http_response_list_t response_list;

	/*http session*/
	http_transaction_list_t transaction_list;
}http_data_t;
#endif
