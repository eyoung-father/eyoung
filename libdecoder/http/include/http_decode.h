#ifndef HTTP_DECODE_H
#define HTTP_DECODE_H 1

#include "libengine_type.h"

typedef void* http_work_t;
typedef void* http_handler_t;

extern http_handler_t http_decoder_init(engine_t engine, void *html_decoder);
extern void http_decoder_finit(http_handler_t handler);

extern http_work_t http_work_create(http_handler_t handler, int greedy);
extern int http_decode_data(http_work_t work, const char *data, size_t data_len, int from_client, int last_frag);
extern void http_work_destroy(http_work_t work);
#endif
