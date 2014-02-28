#ifndef HTTP_DETECT_H
#define HTTP_DETECT_H 1

#include "ey_queue.h"
#include "http_decode.h"
#include "http_type.h"
#include "libengine.h"

typedef struct http_parameter
{
	http_string_t name;
	http_string_t value;
	STAILQ_ENTRY(http_parameter) next;
}http_parameter_t;
typedef STAILQ_HEAD(http_parameter_list, http_parameter) http_parameter_list_t;

extern int http_body_merge(http_work_t work, http_body_t *body, http_string_t *body_string, int from_client);
#define http_client_body_merge(w,b,s) http_body_merge(w,b,s,1)
#define http_server_body_merge(w,b,s) http_body_merge(w,b,s,0)

extern int http_parse_parameter(http_work_t work, http_string_t *in_str, http_parameter_list_t *p_list, 
	char t, char e, int from_client);
#define http_client_parse_parameter(w,i,o,t,e) http_parse_parameter(w,i,o,t,e,1)
#define http_server_parse_parameter(w,i,o,t,e) http_parse_parameter(w,i,o,t,e,0)

extern void http_free_parameter_list(http_work_t work, http_parameter_list_t *p_list, int from_client);
#define http_client_free_parameter_list(w,p) http_free_parameter_list(w,p,1)
#define http_server_free_parameter_list(w,p) http_free_parameter_list(w,p,0)

extern int http_request_uri_preprocessor(engine_work_event_t *event);
extern int http_request_body_preprocessor(engine_work_event_t *event);

extern int http_request_uri_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event);
extern int http_request_body_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event);

#endif
