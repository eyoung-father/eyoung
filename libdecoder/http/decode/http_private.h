#ifndef HTTP_PRIVATE_H
#define HTTP_PRIVATE_H 1

#include "ey_memory.h"
#include "libengine_type.h"
#include "http_type.h"
#include "http_client_parser.h"
#include "http_server_parser.h"
#include "http_client_lex.h"
#include "http_server_lex.h"

struct yy_buffer_state;
extern struct yy_buffer_state* http_client_scan_stream(const char *new_buf, size_t new_buf_len, http_data_t *priv);
extern struct yy_buffer_state* http_server_scan_stream(const char *new_buf, size_t new_buf_len, http_data_t *priv);

extern int parse_http_client_stream(http_data_t *priv, const char *buf, size_t buf_len, int last_frag);
extern int parse_http_server_stream(http_data_t *priv, const char *buf, size_t buf_len, int last_frag);

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
extern int http_element_detect(http_data_t *http_data, const char *event_name, int event_id, void *event,
	char *cluster_buffer, size_t cluster_buffer_len);

/*memory mgt system init api for system initializing*/
extern int http_mem_init(http_decoder_t *decoder);
extern void http_mem_finit(http_decoder_t *decoder);

/*slab mgt api*/
extern http_data_t* http_alloc_priv_data(http_decoder_t *decoder, int greedy);
extern void http_free_priv_data(http_decoder_t *decoder, http_data_t *priv_data);

/*
 * FOR REQUEST
 * */
/*alloc/free request*/
extern http_request_t* http_client_alloc_request(http_decoder_t *decoder,
										  http_request_first_line_t *first_line,
										  http_request_header_list_t *header_list,
										  http_request_body_t *body);
extern void http_client_free_request(http_decoder_t *decoder, http_request_t*request);

/*alloc/free first_line*/
extern http_request_first_line_t *http_client_alloc_first_line(http_decoder_t *decoder,
										  http_request_method_t method,
										  http_request_string_t *uri,
										  http_request_version_t version);
extern void http_client_free_first_line(http_decoder_t *decoder, http_request_first_line_t *first_line);

/*alloc/free header*/
extern http_request_header_t* http_client_alloc_header(http_decoder_t *decoder,
										  http_request_header_type_t type,
										  http_request_string_t *value);
extern void http_client_free_header(http_decoder_t *decoder, http_request_header_t *header);
extern void http_client_free_header_list(http_decoder_t *decoder, http_request_header_list_t *header_list);

/*alloc/free body*/
extern http_request_body_part_t* http_client_alloc_body_part(http_decoder_t *decoder,
										  http_request_string_t *part);
extern void http_client_free_body_part(http_decoder_t *decoder, http_request_body_part_t *part);
extern void http_client_free_body(http_decoder_t *decoder, http_request_body_t *body);

/*
 * FOR RESPONSE
 * */
/*alloc/free response*/
extern http_response_t* http_server_alloc_response(http_decoder_t *decoder,
										  http_response_first_line_t *first_line,
										  http_response_header_list_t *header_list,
										  http_response_body_t *body);
extern void http_server_free_response(http_decoder_t *decoder, http_response_t *response);

/*alloc/free first_line*/
extern http_response_first_line_t *http_server_alloc_first_line(http_decoder_t *decoder,
										  http_response_version_t version,
										  http_response_code_t code,
										  http_response_string_t *message);
extern void http_server_free_first_line(http_decoder_t *decoder, http_response_first_line_t *first_line);

/*alloc/free header*/
extern http_response_header_t* http_server_alloc_header(http_decoder_t *decoder,
										  http_response_header_type_t type,
										  http_response_string_t *value);
extern void http_server_free_header(http_decoder_t *decoder, http_response_header_t *header);
extern void http_server_free_header_list(http_decoder_t *decoder, http_response_header_list_t *header_list);

/*alloc/free body*/
extern http_response_body_part_t* http_server_alloc_body_part(http_decoder_t *decoder,
										  http_response_string_t *part);
extern void http_server_free_body_part(http_decoder_t *decoder, http_response_body_part_t *part);
extern void http_server_free_body(http_decoder_t *decoder, http_response_body_t *body);
#endif
