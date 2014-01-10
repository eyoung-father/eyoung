#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "http.h"
#include "http_private.h"
#include "http_client_parser.h"
#include "http_server_parser.h"
#include "http_client_lex.h"
#include "http_server_lex.h"

extern int http_cmd_pair_id;

int http_mem_init(http_decoder_t *decoder)
{
	decoder->http_data_slab = http_zinit("http private data", sizeof(http_data_t));
	if(!decoder->http_data_slab)
	{
		http_debug(debug_http_mem, "init http_data_slab failed\n");
		return -1;
	}

	decoder->http_request_slab = http_zinit("http request data", sizeof(http_request_t));
	if(!decoder->http_request_slab)
	{
		http_debug(debug_http_mem, "init http_request_slab failed\n");
		return -1;
	}

	decoder->http_response_slab = http_zinit("http response data", sizeof(http_response_t));
	if(!decoder->http_response_slab)
	{
		http_debug(debug_http_mem, "init http_response_slab failed\n");
		return -1;
	}

	decoder->http_cmd_slab = http_zinit("http command", sizeof(http_cmd_t));
	if(!decoder->http_cmd_slab)
	{
		http_debug(debug_http_mem, "init http_cmd_slab failed\n");
		return -1;
	}

	return 0;
}

void http_mem_finit(http_decoder_t *decoder)
{
	http_zfinit(decoder->http_data_slab);
	http_zfinit(decoder->http_request_slab);
	http_zfinit(decoder->http_response_slab);
	http_zfinit(decoder->http_cmd_slab);
}

http_data_t* http_alloc_priv_data(http_decoder_t *decoder, int greedy)
{
	void *client_lexier = NULL;
	void *client_parser = NULL;
	void *server_lexier = NULL;
	void *server_parser = NULL;
	http_data_t *data = NULL;

	client_parser = (void*)http_client_pstate_new();
	if(!client_parser)
	{
		http_debug(debug_http_mem, "alloc client parser failed\n");
		goto failed;
	}

	if(http_client_lex_init(&client_lexier))
	{
		http_debug(debug_http_mem, "alloc client lexier failed\n");
		goto failed;
	}

	server_parser = (void*)http_server_pstate_new();
	if(!server_parser)
	{
		http_debug(debug_http_mem, "alloc server parser failed\n");
		goto failed;
	}

	if(http_server_lex_init(&server_lexier))
	{
		http_debug(debug_http_mem, "alloc server lexier failed\n");
		goto failed;
	}
	
	data = (http_data_t*)http_zalloc(decoder->http_data_slab);
	if(!data)
	{
		http_debug(debug_http_mem, "alloc private data failed\n");
		goto failed;
	}

	memset(data, 0, sizeof(*data));

	/*init client*/
	data->request_parser.parser = client_parser;
	data->request_parser.lexier = client_lexier;
	data->request_parser.greedy = greedy;
	STAILQ_INIT(&data->request_list);
	http_client_set_extra((void*)data, client_lexier);

	/*init server*/
	data->response_parser.parser = server_parser;
	data->response_parser.lexier = server_lexier;
	data->response_parser.greedy = greedy;
	STAILQ_INIT(&data->response_list);
	http_server_set_extra((void*)data, server_lexier);

	/*init session*/
	STAILQ_INIT(&data->cmd_list);
	return data;

failed:
	if(client_parser)
		http_client_pstate_delete((http_client_pstate*)client_parser);
	if(client_lexier)
		http_client_lex_destroy(client_lexier);
	if(server_parser)
		http_server_pstate_delete((http_server_pstate*)server_parser);
	if(server_lexier)
		http_server_lex_destroy(server_lexier);
	if(data)
		http_zfree(decoder->http_data_slab, data);
	return NULL;
}

void http_free_priv_data(http_decoder_t *decoder, http_data_t *data)
{
	if(!data)
		return;
	
	/*free client*/
	if(data->request_parser.parser)
		http_client_pstate_delete((http_client_pstate*)data->request_parser.parser);
	if(data->request_parser.lexier)
		http_client_lex_destroy(data->request_parser.lexier);
	if(data->request_parser.saved)
		http_free(data->request_parser.saved);

	/*free server*/
	if(data->response_parser.parser)
		http_server_pstate_delete((http_server_pstate*)data->response_parser.parser);
	if(data->response_parser.lexier)
		http_server_lex_destroy(data->response_parser.lexier);
	if(data->response_parser.saved)
		http_free(data->response_parser.saved);

	/*free private data self*/
	http_zfree(decoder->http_data_slab, data);
}
