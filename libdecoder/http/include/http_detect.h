#ifndef HTTP_DETECT_H
#define HTTP_DETECT_H 1

#include "ey_queue.h"
#include "http_decode.h"
#include "http_type.h"
#include "libengine.h"

typedef struct parameter
{
	http_string_t name;
	http_string_t value;
	STAILQ_ENTRY(parameter) next;
}parameter_t;
typedef STAILQ_HEAD(parameter_list, parameter) parameter_list_t;

typedef struct request_data
{
	http_string_t raw_data;
	parameter_list_t parameter_list;
}request_data_t;

extern int http_request_body_init(engine_work_event_t *event);
extern int http_request_body_finit(engine_work_event_t *event);
extern int http_request_uri_preprocessor(engine_work_event_t *event);
extern int http_request_body_preprocessor(engine_work_event_t *event);

extern int http_request_uri_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event);
extern int http_request_body_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event);

#endif
