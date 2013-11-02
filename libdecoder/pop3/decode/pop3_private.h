#ifndef POP3_PRIVATE_H
#define POP3_PRIVATE_H 1

struct yy_buffer_state;
struct pop3_data;

extern struct yy_buffer_state* pop3_client_scan_stream(const char *new_buf, size_t new_buf_len, struct pop3_data *priv);
extern struct yy_buffer_state* pop3_server_scan_stream(const char *new_buf, size_t new_buf_len, struct pop3_data *priv);

extern int parse_pop3_client_stream(struct pop3_data *priv, const char *buf, size_t buf_len, int last_frag);
extern int parse_pop3_server_stream(struct pop3_data *priv, const char *buf, size_t buf_len, int last_frag);

#include "ey_memory.h"
#include "libengine_type.h"

typedef struct pop3_decoder
{
	engine_t engine;
	ey_slab_t pop3_data_slab;
	ey_slab_t pop3_request_slab;
	ey_slab_t pop3_response_slab;
	ey_slab_t pop3_cmd_slab;
	ey_slab_t pop3_req_arg_slab;
	ey_slab_t pop3_res_line_slab;
}pop3_decoder_t;

extern void pop3_server_register(pop3_decoder_t *decoder);
extern void pop3_client_register(pop3_decoder_t *decoder);
extern int pop3_element_detect(struct pop3_data *pop3_data, const char *event_name, int event_id, void *event,
	char *cluster_buffer, size_t cluster_buffer_len);
#endif
