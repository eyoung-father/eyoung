#ifndef HTTP_TYPE_H
#define HTTP_TYPE_H 1

#include "ey_queue.h"
#include "http_decode.h"
#include "libengine.h"

/*
 * Client Message Type
 */
typedef int http_request_type_t;

typedef struct http_request
{
	http_request_type_t req_code;
	STAILQ_ENTRY(http_request) next;
}http_request_t;

typedef STAILQ_HEAD(http_requests, http_request) http_requests_t;

/*
 * Server Message Type
 */
typedef int http_response_type_t;

typedef struct http_response
{
	http_response_type_t res_code;
	STAILQ_ENTRY(http_response) next;
}http_response_t;

typedef STAILQ_HEAD(http_responses, http_response) http_responses_t;

/*
 * http session command
 */
typedef struct http_cmd
{
	http_request_t *req;
	http_response_t *res;
	STAILQ_ENTRY(http_cmd) next;
}http_cmd_t;

typedef STAILQ_HEAD(http_cmd_list, http_cmd) http_cmd_list_t;

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
	http_requests_t request_list;

	/*server ==> client*/
	http_parser_t response_parser;
	http_responses_t response_list;

	/*http session*/
	http_cmd_list_t cmd_list;
}http_data_t;
#endif
