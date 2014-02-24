#include <string.h>
#include "html.h"
#include "html_private.h"
#include "html_parser.h"
#include "html_lex.h"

/*for memory system init*/
int html_mem_init(html_decoder_t *decoder)
{
	decoder->html_data_slab = html_zinit("html data", sizeof(html_data_t), 0, 0, 0);
	if(!decoder->html_data_slab)
	{
		html_debug(debug_html_mem, "init html private data slab failed\n");
		return -1;
	}

	decoder->html_node_slab = html_zinit("html node", sizeof(html_node_t), 0, 0, 0);
	if(!decoder->html_node_slab)
	{
		html_debug(debug_html_mem, "init html node slab failed\n");
		return -1;
	}

	decoder->html_node_prot_slab = html_zinit("html node prot", sizeof(html_node_prot_t), 0, 0, 0);
	if(!decoder->html_node_prot_slab)
	{
		html_debug(debug_html_mem, "init html node prot slab failed\n");
		return -1;
	}

	decoder->html_value_fslab = html_fzinit("html value", DEFAULT_VALUE_LENGTH);
	if(decoder->html_value_fslab)
	{
		html_debug(debug_html_mem, "init html value fslab failed\n");
		return -1;
	}

	return 0;
}

void html_mem_finit(html_decoder_t *decoder)
{
	if(!decoder)
		return;
	html_zfinit(decoder->html_data_slab);
	html_zfinit(decoder->html_node_slab);
	html_zfinit(decoder->html_node_prot_slab);
	html_fzfinit(decoder->html_value_fslab);
}

/*
 * HTML_NODE_T malloc/free
 * */
html_node_t* html_alloc_node(html_decoder_t *decoder)
{
	html_node_t *ret = (html_node_t*)html_zalloc(decoder->html_node_slab);
	if(!ret)
	{
		html_debug(debug_html_mem, "alloc html node failed\n");
		return NULL;
	}
	
	memset(ret, 0, sizeof(*ret));
	TAILQ_INIT(&ret->prot);
	TAILQ_INIT(&ret->child);
	return ret;
}

void html_free_node(html_decoder_t *decoder, html_node_t *node)
{
	if(!node)
	{
		html_debug(debug_html_mem, "null node in %s\n", __FUNCTION__);
		return;
	}

	html_free_string(decoder, &node->text);
	html_free_node_list(decoder, &node->child);
	html_free_prot_list(decoder, &node->prot);

	html_zfree(decoder->html_node_slab, node);
}

void html_free_node_list(html_decoder_t *decoder, html_node_list_t *node_list)
{
	if(!node_list)
	{
		html_debug(debug_html_mem, "null node list in %s\n", __FUNCTION__);
		return;
	}

	html_node_t *node=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(node, node_list, sib, tmp)
		html_free_node(decoder, node);
	TAILQ_INIT(node_list);
}

/*
 * HTML_NODE_PROT_T malloc/free
 * */
html_node_prot_t* html_alloc_prot(html_decoder_t *decoder)
{
	html_node_prot_t *ret = (html_node_prot_t*)html_zalloc(decoder->html_node_prot_slab);
	if(!ret)
	{
		html_debug(debug_html_mem, "alloc html node prot failed\n");
		return NULL;
	}

	memset(ret, 0, sizeof(*ret));
	return ret;
}

void html_free_prot(html_decoder_t *decoder, html_node_prot_t *prot)
{
	if(!prot)
	{
		html_debug(debug_html_mem, "null prot in %s\n", __FUNCTION__);
		return;
	}
	html_free_string(decoder, &prot->value);
	html_zfree(decoder->html_node_prot_slab, prot);
}

void html_free_prot_list(html_decoder_t *decoder, html_node_prot_list_t *prot_list)
{
	if(!prot_list)
	{
		html_debug(debug_html_mem, "null prot list in %s\n", __FUNCTION__);
		return;
	}

	html_node_prot_t *prot=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(prot, prot_list, next, tmp)
		html_free_prot(prot);
	TAILQ_INIT(prot_list);
}

/*
 * HTML_DATA_T malloc/free
 * */
html_data_t *html_alloc_priv_data(html_decoder_t *decoder, int greedy, int create_dom)
{
	void *lexier = NULL;
	void *parser = NULL;
	htmp_data_t *data = NULL;

	parser = (void*)html_pstate_new();
	if(!parser)
	{
		html_debug(debug_html_mem, "alloc html parser failed\n");
		goto failed;
	}

	if(html_lex_init(&lexier))
	{
		html_debug(debug_html_mem, "alloc html lexier failed\n");
		goto failed;
	}

	data = (html_data_t*)html_zalloc(decoder->html_data_slab);
	if(!data)
	{
		html_debug(debug_html_mem, "alloc html private data failed\n");
		goto failed;
	}

	memset(data, 0, sizeof(*data));
	data->decoder = (html_handler_t)decoder;

	data->parser.parser = parser;
	data->parser.lexier = lexier;
	data->parser.greedy = greedy;
	TAILQ_INIT(&data->html_root);
	data->create_dom = create_dom;
	html_set_extra((void*)data, lexier);

	return data;

failed:
	if(parser)
		html_pstate_delete((html_pstate*)parser);
	if(lexier)
		html_lex_destroy(lexier);
	if(data)
		html_zfree(decoder->html_data_slab, data);
	return NULL;
}

void html_free_priv_data(html_decoder_t *decoder, html_data_t *data)
{
	if(!data)
	{
		html_debug(debug_html_mem, "null html private data\n");
		return;
	}
	
	if(data->parser.parser)
		html_pstate_delete((html_pstate*)data->parser.parser);
	if(data->parser.lexier)
		html_lex_destroy(data->parser.lexier);
	if(data->parser.saved)
		html_free(data->parser.saved);
	html_free_node_list(decoder, &data->html_root);

	html_zfree(decoder->html_data_slab, data);
}

/*
 * HTML_STRING malloc/free
 * */
char* html_alloc_string(html_decoder_t *decoder, const char *i_str, size_t i_len, html_string_t *o_str)
{
	if(!o_str)
	{
		html_debug(debug_html_mem, "null output html string\n");
		return NULL;
	}
	
	char *ret = (char*)html_fzalloc(i_len+1, decoder->htmlvalue_fslab);
	if(ret)
	{
		if(i_str)
			memcpy(ret, i_str, i_len);
		ret[i_len] = '\0';
		o_str->buf = ret;
		o_str->len = i_len;
	}
	return ret;
}

void html_free_string(html_decoder_t *decoder, html_string_t *string)
{
	if(!string || !string->buf)
	{
		html_debug(debug_html_mem, "null html string to free\n");
		return;
	}

	html_fzfree(decoder->html_value_fslab, string->buf);
	string->buf = NULL;
	string->len = 0;
}
