#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "html.h"
#include "html_private.h"
#include "libengine.h"

html_work_t html_work_create(html_handler_t handler, int greedy)
{
	if(!handler)
	{
		ey_html_debug(debug_html_mem, "null html decoder handler\n");
		return NULL;
	}
	
	html_data_t *priv_data = NULL;
	html_decoder_t *decoder = (html_decoder_t*)handler;
	engine_t engine = decoder->engine;
	priv_data = html_alloc_priv_data(decoder, greedy, engine!=NULL);
	if(!priv_data)
	{
		ey_html_debug(debug_html_mem, "failed to alloc html private data\n");
		return NULL;
	}

	priv_data->decoder = handler;
	if(engine)
	{
		engine_work_t *engine_work = ey_engine_work_create(engine);
		if(!engine_work)
		{
			ey_html_debug(debug_html_mem, "failed to alloc engine work\n");
			html_free_priv_data(decoder, priv_data);
			return NULL;
		}
		priv_data->engine_work = engine_work;
		engine_work->predefined = (void*)priv_data;
	}
	
	return priv_data;
}

void html_work_destroy(html_work_t work)
{
	if(work)
	{
		html_data_t *priv_data = (html_data_t*)work;
		html_decoder_t *decoder = (html_decoder_t*)(priv_data->decoder);
		if(priv_data->engine_work)
			ey_engine_work_destroy(priv_data->engine_work);
		html_free_priv_data(decoder, priv_data);
	}
}

int html_decode_data(html_work_t work, const char *data, size_t data_len, int last_frag)
{
	assert(work != NULL);
	return parse_html_stream((html_data_t*)work, data, data_len, last_frag);
}

html_handler_t html_decoder_init(engine_t engine)
{
	html_decoder_t *decoder = NULL;
	decoder = (html_decoder_t*)ey_html_malloc(sizeof(html_decoder_t));
	if(!decoder)
	{
		ey_html_debug(debug_html_mem, "failed to alloc html decoder\n");
		goto failed;
	}
	memset(decoder, 0, sizeof(*decoder));

	if(html_mem_init(decoder))
	{
		ey_html_debug(debug_html_mem, "failed to init html decoder mem\n");
		goto failed;
	}
	
	if(engine)
	{
		decoder->engine = engine;
		html_register(decoder);
	}
	return (html_handler_t)decoder;

failed:
	if(decoder)
		html_decoder_finit((html_handler_t)decoder);
	return NULL;
}

void html_decoder_finit(html_handler_t handler)
{
	if(handler)
	{
		html_decoder_t *decoder = (html_decoder_t*)handler;
		html_mem_finit(decoder);
		ey_html_free(decoder);
	}
}

int html_element_detect(html_data_t *html_data, const char *event_name, int event_id, void *event, 
	char *cluster_buffer, size_t cluster_buffer_len)
{
	if(!event_name)
	{
		ey_html_debug(debug_html_detect, "event name is null\n");
		return 0;
	}

	if(event_id < 0)
	{
		ey_html_debug(debug_html_detect, "event id %d for event %s is illegal\n", event_id, event_name);
		return 0;
	}

	if(!html_data || !event)
	{
		ey_html_debug(debug_html_detect, "bad parameter for event %s\n", event_name);
		return 0;
	}

	engine_action_t action = {ENGINE_ACTION_PASS};
	engine_work_t *work = html_data->engine_work;
	engine_work_event_t *work_event = ey_engine_work_create_event(work, event_id, &action);
	if(!work_event)
	{
		ey_html_debug(debug_html_detect, "create event for %s failed\n", event_name);
		return 0;
	}

	ey_engine_work_set_data(work_event, event, cluster_buffer, cluster_buffer_len);
	ey_engine_work_detect_event(work_event);
	ey_engine_work_destroy_event(work_event);
	ey_html_debug(debug_html_detect, "detect %s[%d], get actoin %s\n",
		event_name, event_id, ey_engine_action_name(action.action));
	if(action.action==ENGINE_ACTION_PASS)
		return 0;
	return -1;
}
