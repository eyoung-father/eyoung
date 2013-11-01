#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pop3_mem.h"
#include "pop3_util.h"
#include "pop3_type.h"
#include "pop3_private.h"

pop3_work_t pop3_work_create(pop3_handler_t handler, int greedy)
{
	pop3_data_t *priv_data = NULL;
	pop3_decoder_t *decoder = (pop3_decoder_t*)handler;
	if(!handler)
	{
		pop3_debug(debug_pop3_mem, "null pop3 decoder handler\n");
		return NULL;
	}
	
	priv_data = pop3_alloc_priv_data(decoder, greedy);
	if(!priv_data)
	{
		pop3_debug(debug_pop3_mem, "failed to alloc pop3 private data\n");
		return NULL;
	}
	
	return priv_data;
}

void pop3_work_destroy(pop3_work_t work)
{
	if(work)
	{
		pop3_data_t *priv_data = (pop3_data_t*)work;
		pop3_decoder_t *decoder = (pop3_decoder_t*)(priv_data->decoder);
		pop3_free_priv_data(decoder, priv_data);
	}
}

int pop3_decode_data(pop3_work_t work, const char *data, size_t data_len, int from_client, int last_frag)
{
	assert(work != NULL);
	if(from_client)
		return parse_pop3_client_stream((pop3_data_t*)work, data, data_len, last_frag);
	else
		return parse_pop3_server_stream((pop3_data_t*)work, data, data_len, last_frag);
}

pop3_handler_t pop3_decoder_init()
{
	pop3_decoder_t *decoder = NULL;
	decoder = (pop3_decoder_t*)pop3_malloc(sizeof(pop3_decoder_t));
	if(!decoder)
	{
		pop3_debug(debug_pop3_mem, "failed to alloc pop3 decoder\n");
		goto failed;
	}
	memset(decoder, 0, sizeof(*decoder));

	if(pop3_mem_init(decoder))
	{
		pop3_debug(debug_pop3_mem, "failed to init pop3 decoder mem\n");
		goto failed;
	}

	return (pop3_handler_t)decoder;

failed:
	if(decoder)
		pop3_decoder_finit((pop3_handler_t)decoder);
	return NULL;
}

void pop3_decoder_finit(pop3_handler_t handler)
{
	if(handler)
	{
		pop3_decoder_t *decoder = (pop3_decoder_t*)handler;
		pop3_mem_finit(decoder);
		pop3_free(decoder);
	}
}
