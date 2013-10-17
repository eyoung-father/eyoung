#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pop3_mem.h"
#include "pop3_util.h"
#include "pop3_type.h"
#include "pop3_decode.h"
#include "pop3_private.h"

pop3_decode_t pop3_decode_create(int greedy)
{
	pop3_data_t *priv_data = NULL;
	
	priv_data = pop3_alloc_priv_data(greedy);
	if(!priv_data)
	{
		pop3_debug(debug_pop3_mem, "failed to alloc pop3 private data\n");
		return NULL;
	}
	
	return priv_data;
}

void pop3_decode_destroy(pop3_decode_t decode)
{
	if(decode)
		pop3_free_priv_data((pop3_data_t*)decode);
}

int pop3_decode_data(pop3_decode_t decoder, const char *data, size_t data_len, int from_client, int last_frag)
{
	assert(decoder != NULL);
	if(from_client)
		return parse_pop3_client_stream((pop3_data_t*)decoder, data, data_len, last_frag);
	else
		return parse_pop3_server_stream((pop3_data_t*)decoder, data, data_len, last_frag);
}

void pop3_decode_init()
{
	pop3_mem_init();
}

void pop3_decode_finit()
{
	pop3_mem_finit();
}
