#ifndef HTTP_PRIVATE_H
#define HTTP_PRIVATE_H 1

struct yy_buffer_state;
struct http_data;

extern struct yy_buffer_state* http_client_scan_stream(const char *new_buf, size_t new_buf_len, struct http_data *priv);
extern struct yy_buffer_state* http_server_scan_stream(const char *new_buf, size_t new_buf_len, struct http_data *priv);

extern int parse_http_client_stream(struct http_data *priv, const char *buf, size_t buf_len, int last_frag);
extern int parse_http_server_stream(struct http_data *priv, const char *buf, size_t buf_len, int last_frag);

#include "ey_memory.h"
#include "libengine_type.h"

typedef struct http_decoder
{
	engine_t engine;
	ey_slab_t http_data_slab;
	ey_slab_t http_request_slab;
	ey_slab_t http_response_slab;
	ey_slab_t http_cmd_slab;
}http_decoder_t;

extern void http_server_register(http_decoder_t *decoder);
extern void http_client_register(http_decoder_t *decoder);
extern int http_element_detect(struct http_data *http_data, const char *event_name, int event_id, void *event,
	char *cluster_buffer, size_t cluster_buffer_len);

/*memory mgt system init api for system initializing*/
extern int http_mem_init(http_decoder_t *decoder);
extern void http_mem_finit(http_decoder_t *decoder);

/*slab mgt api*/
struct http_data;
extern struct http_data* http_alloc_priv_data(http_decoder_t *decoder, int greedy);
extern void http_free_priv_data(http_decoder_t *decoder, struct http_data *priv_data);
#endif
