#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "http.h"
#include "http_private.h"
#include "libengine.h"

http_work_t http_work_create(http_handler_t handler, int greedy)
{
	if(!handler)
	{
		http_debug(debug_http_mem, "null http decoder handler\n");
		return NULL;
	}
	
	http_data_t *priv_data = NULL;
	http_decoder_t *decoder = (http_decoder_t*)handler;
	engine_t engine = decoder->engine;
	priv_data = http_alloc_priv_data(decoder, greedy);
	if(!priv_data)
	{
		http_debug(debug_http_mem, "failed to alloc http private data\n");
		return NULL;
	}

	priv_data->decoder = handler;
	if(engine)
	{
		engine_work_t *engine_work = ey_engine_work_create(engine);
		if(!engine_work)
		{
			http_debug(debug_http_mem, "failed to alloc engine work\n");
			http_free_priv_data(decoder, priv_data);
			return NULL;
		}
		priv_data->engine_work = engine_work;
		engine_work->predefined = (void*)priv_data;
	}
	
	return priv_data;
}

void http_work_destroy(http_work_t work)
{
	if(work)
	{
		http_data_t *priv_data = (http_data_t*)work;
		http_decoder_t *decoder = (http_decoder_t*)(priv_data->decoder);
		if(priv_data->engine_work)
			ey_engine_work_destroy(priv_data->engine_work);
		http_free_priv_data(decoder, priv_data);
	}
}

int http_decode_data(http_work_t work, const char *data, size_t data_len, int from_client, int last_frag)
{
	assert(work != NULL);
	if(from_client)
		return parse_http_client_stream((http_data_t*)work, data, data_len, last_frag);
	else
		return parse_http_server_stream((http_data_t*)work, data, data_len, last_frag);
}

http_handler_t http_decoder_init(engine_t engine, void *html_decoder)
{
	http_decoder_t *decoder = NULL;
	decoder = (http_decoder_t*)http_malloc(sizeof(http_decoder_t));
	if(!decoder)
	{
		http_debug(debug_http_mem, "failed to alloc http decoder\n");
		goto failed;
	}
	memset(decoder, 0, sizeof(*decoder));

	if(http_mem_init(decoder))
	{
		http_debug(debug_http_mem, "failed to init http decoder mem\n");
		goto failed;
	}
	
	decoder->html_decoder = html_decoder;
	if(engine)
	{
		decoder->engine = engine;
		http_server_register(decoder);
		http_client_register(decoder);
	}
	return (http_handler_t)decoder;

failed:
	if(decoder)
		http_decoder_finit((http_handler_t)decoder);
	return NULL;
}

void http_decoder_finit(http_handler_t handler)
{
	if(handler)
	{
		http_decoder_t *decoder = (http_decoder_t*)handler;
		http_mem_finit(decoder);
		http_free(decoder);
	}
}

int http_element_detect(http_data_t *http_data, const char *event_name, int event_id, void *event, 
	char *cluster_buffer, size_t cluster_buffer_len)
{
	if(!http_data->engine_work)
	{
		http_debug(debug_http_detect, "engine work is not created, skip scan\n");
		return 0;
	}

	if(!event_name)
	{
		http_debug(debug_http_detect, "event name is null\n");
		return 0;
	}

	if(event_id < 0)
	{
		http_debug(debug_http_detect, "event id %d for event %s is illegal\n", event_id, event_name);
		return 0;
	}

	if(!http_data || !event)
	{
		http_debug(debug_http_detect, "bad parameter for event %s\n", event_name);
		return 0;
	}

	engine_action_t action = {ENGINE_ACTION_PASS};
	engine_work_t *work = http_data->engine_work;
	engine_work_event_t *work_event = ey_engine_work_create_event(work, event_id, &action);
	if(!work_event)
	{
		http_debug(debug_http_detect, "create event for %s failed\n", event_name);
		return 0;
	}
	ey_engine_work_set_data(work_event, event, cluster_buffer, cluster_buffer_len);
	ey_engine_work_detect_event(work_event);
	ey_engine_work_destroy_event(work_event);
	http_debug(debug_http_detect, "detect %s[%d], get actoin %s\n",
		event_name, event_id, ey_engine_action_name(action.action));
	if(action.action==ENGINE_ACTION_PASS)
		return 0;
	return -1;
}
