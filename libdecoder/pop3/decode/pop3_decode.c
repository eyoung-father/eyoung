#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pop3_mem.h"
#include "pop3_util.h"
#include "pop3_type.h"
#include "pop3_private.h"
#include "libengine.h"

pop3_work_t pop3_work_create(pop3_handler_t handler, int greedy)
{
	if(!handler)
	{
		pop3_debug(debug_pop3_mem, "null pop3 decoder handler\n");
		return NULL;
	}
	
	pop3_data_t *priv_data = NULL;
	pop3_decoder_t *decoder = (pop3_decoder_t*)handler;
	engine_t engine = decoder->engine;
	priv_data = pop3_alloc_priv_data(decoder, greedy);
	if(!priv_data)
	{
		pop3_debug(debug_pop3_mem, "failed to alloc pop3 private data\n");
		return NULL;
	}

	engine_work_t *engine_work = ey_engine_work_create(engine);
	if(!engine_work)
	{
		pop3_debug(debug_pop3_mem, "failed to alloc engine work\n");
		pop3_free_priv_data(decoder, priv_data);
		return NULL;
	}
	priv_data->engine_work = engine_work;
	priv_data->decoder = handler;
	engine_work->predefined = (void*)priv_data;
	
	return priv_data;
}

void pop3_work_destroy(pop3_work_t work)
{
	if(work)
	{
		pop3_data_t *priv_data = (pop3_data_t*)work;
		pop3_decoder_t *decoder = (pop3_decoder_t*)(priv_data->decoder);
		if(priv_data->engine_work)
			ey_engine_work_destroy(priv_data->engine_work);
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

pop3_handler_t pop3_decoder_init(engine_t engine)
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

	decoder->engine = engine;
	pop3_server_register(decoder);
	pop3_client_register(decoder);
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

int pop3_element_detect(pop3_data_t *pop3_data, const char *event_name, int event_id, void *event, 
	char *cluster_buffer, size_t cluster_buffer_len)
{
	if(!event_name)
	{
		pop3_debug(debug_pop3_detect, "event name is null\n");
		return 0;
	}

	if(event_id < 0)
	{
		pop3_debug(debug_pop3_detect, "event id %d for event %s is illegal\n", event_id, event_name);
		return 0;
	}

	if(!pop3_data || !event)
	{
		pop3_debug(debug_pop3_detect, "bad parameter for event %s\n", event_name);
		return 0;
	}

	engine_action_t action = {ENGINE_ACTION_PASS};
	engine_work_t *work = pop3_data->engine_work;
	engine_work_event_t *work_event = ey_engine_work_create_event(work, event_id, &action);
	if(!work_event)
	{
		pop3_debug(debug_pop3_detect, "create event for %s failed\n", event_name);
		return 0;
	}
	ey_engine_work_set_data(work_event, event, cluster_buffer, cluster_buffer_len);
	ey_engine_work_detect_event(work_event);
	ey_engine_work_destroy_event(work_event);
	pop3_debug(debug_pop3_detect, "detect %s[%d], get actoin %s\n",
		event_name, event_id, ey_engine_action_name(action.action));
	if(action.action==ENGINE_ACTION_PASS)
		return 0;
	return -1;
}
