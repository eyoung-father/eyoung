#ifndef HTML_PRIVATE_H
#define HTML_PRIVATE_H 1

#include "ey_memory.h"
#include "libengine_type.h"
#include "html_type.h"
#include "html_parser.h"

struct yy_buffer_state;
extern struct yy_buffer_state* html_scan_stream(const char *new_buf, size_t new_buf_len, html_data_t *priv);
extern int parse_html_stream(html_data_t *priv, const char *buf, size_t buf_len, int last_frag);

#define DEFAULT_VALUE_LENGTH	128
typedef struct html_decoder
{
	engine_t engine;

	ey_fslab_t html_value_fslab;
	ey_slab_t html_data_slab;
	ey_slab_t html_node_slab;
	ey_slab_t html_node_prot_slab;
}html_decoder_t;

extern void html_register(html_decoder_t *decoder);
extern int html_element_detect(html_data_t *html_data, const char *event_name, int event_id, void *event,
	char *cluster_buffer, size_t cluster_buffer_len);

extern int html_mem_init(html_decoder_t *decoder);
extern void html_mem_finit(html_decoder_t *decoder);

/*
 * HTML_NODE_T malloc/free
 * */
extern html_node_t* html_alloc_node(html_decoder_t *decoder);
extern void html_free_node(html_decoder_t *decoder, html_node_t *node);
extern void html_free_node_list(html_decoder_t *decoder, html_node_list_t *node_list);

/*
 *HTML_NODE_PROT_T malloc/free
 * */
extern html_node_prot_t* html_alloc_prot(html_decoder_t *decoder);
extern void html_free_prot(html_decoder_t *decoder, html_node_prot_t *prot);
extern void html_free_prot_list(html_decoder_t *decoder, html_node_prot_list_t *prot_list);

/*
 * HTML_DATA_T malloc/free
 * */
extern html_data_t *html_alloc_priv_data(html_decoder_t *decoder, int greedy, int create_dom);
extern void html_free_priv_data(html_decoder_t *decoder, html_data_t *data);
#endif
