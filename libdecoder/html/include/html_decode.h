#ifndef HTML_DECODE_H
#define HTML_DECODE_H 1

#include "libengine_type.h"

typedef void* html_work_t;
typedef void* html_handler_t;

extern html_handler_t html_decoder_init(engine_t engine);
extern void html_decoder_finit(html_handler_t handler);

extern html_work_t html_work_create(html_handler_t handler, int greedy);
extern int html_decode_data(html_work_t work, const char *data, size_t data_len, int last_frag);
extern void html_work_destroy(html_work_t work);
#endif