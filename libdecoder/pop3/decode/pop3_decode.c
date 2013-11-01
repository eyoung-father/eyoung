#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pop3_mem.h"
#include "pop3_util.h"
#include "pop3_type.h"
#include "pop3_decode.h"
#include "pop3_private.h"

pop3_work_t pop3_work_create(int greedy)
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

void pop3_work_destroy(pop3_work_t work)
{
	if(work)
		pop3_free_priv_data((pop3_data_t*)work);
}

int pop3_decode_data(pop3_work_t work, const char *data, size_t data_len, int from_client, int last_frag)
{
	assert(work != NULL);
	if(from_client)
		return parse_pop3_client_stream((pop3_data_t*)work, data, data_len, last_frag);
	else
		return parse_pop3_server_stream((pop3_data_t*)work, data, data_len, last_frag);
}

void pop3_decoder_init()
{
	pop3_mem_init();
}

void pop3_decoder_finit()
{
	pop3_mem_finit();
}
