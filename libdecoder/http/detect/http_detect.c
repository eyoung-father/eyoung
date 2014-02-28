#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "http.h"
#include "http_detect.h"
#include "http_private.h"

static void http_free_parameter(http_decoder_t *decoder, http_parameter_t *pm, int from_client)
{
	assert(decoder != NULL);

	if(!pm)
	{
		http_debug(debug_http_detect, "%s: null paramter data\n", __FUNCTION__);
		return;
	}
	http_free_string(decoder, &pm->name, from_client);
	http_free_string(decoder, &pm->value, from_client);
	http_free(pm);
}

void http_free_parameter_list(http_work_t work, http_parameter_list_t *p_list, int from_client)
{
	assert(work != NULL);
	http_data_t *data = (http_data_t*)work;
	http_decoder_t *decoder = (http_decoder_t*)data->decoder;

	http_parameter_t *pm=NULL, *tmp=NULL;
	STAILQ_FOREACH_SAFE(pm, p_list, next, tmp)
		http_free_parameter(decoder,pm,from_client);
}

int http_body_merge(http_work_t work, http_body_t *body, http_string_t *body_string, int from_client)
{
	assert(work != NULL);
	assert(body_string != NULL);
	http_data_t *data = (http_data_t*)work;
	http_decoder_t *decoder = (http_decoder_t*)data->decoder;
	char *wt = NULL, *o_buf=NULL;
	size_t o_len = 0;
	
	if(!body)
	{
		http_debug(debug_http_detect, "%s: null body parameter\n", __FUNCTION__);
		return -1;
	}

	o_len = body->info.body_size;
	o_buf = http_alloc_string(decoder, NULL, o_len, body_string, from_client);
	if(!o_buf)
	{
		http_debug(debug_http_detect, "%s: failed to alloc merged body space\n", __FUNCTION__);
		return -1;
	}

	wt = o_buf;
	if(body->info.transfer_encoding == HTTP_BODY_TRANSFER_ENCODING_CHUNKED)
	{
		http_chunk_body_part_t *chunk_part = NULL;
		STAILQ_FOREACH(chunk_part, &body->chunk_body.chunk_list, next)
		{
			http_string_list_t *chunk_value_list = &chunk_part->chunk_value;
			http_string_list_part_t *part = NULL;
			STAILQ_FOREACH(part, chunk_value_list, next)
			{
				if(!part->string.len)
					continue;

				assert(wt + part->string.len <= o_buf + o_len);
				memcpy(wt, part->string.buf, part->string.len);
				wt += part->string.len;
			}
		}
	}
	else
	{
		http_string_list_part_t *part = NULL;
		STAILQ_FOREACH(part, &body->normal_body, next)
		{
			if(!part->string.len)
				continue;
			assert(wt + part->string.len <= o_buf + o_len);
			memcpy(wt, part->string.buf, part->string.len);
			wt += part->string.len;
		}
	}

	*wt = '\0';
	return 0;
}

int http_request_uri_preprocessor(engine_work_event_t *event)
{
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	http_debug(debug_http_detect, ">>>>>>>>>>>>>>>>>%s return 0\n", __FUNCTION__);
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 0;
}

int http_request_body_init(engine_work_event_t *event)
{
	http_string_t *body = NULL;
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	body = (http_string_t*)http_malloc(sizeof(http_string_t));
	if(!body)
	{
		http_debug(debug_http_detect, ">>>>>>>>>>>>>>>>>%s return 1\n", __FUNCTION__);
		http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
		event->user_defined = body;
		return 1;
	}
	memset(body, 0, sizeof(*body));
	event->user_defined = body;
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 0;
}

int http_request_body_finit(engine_work_event_t *event)
{
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	if(event->user_defined)
	{
		engine_work_t *engine_work = event->work;
		http_data_t *http_data = (http_data_t*)engine_work->predefined;
		http_decoder_t *http_decoder = (http_decoder_t*)http_data->decoder;
		http_string_t *body = (http_string_t*)event->user_defined;
		
		http_free_string(http_decoder, body, 1);
		http_free(body);
		event->user_defined = NULL;
	}
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 0;
}

int http_request_body_preprocessor(engine_work_event_t *event)
{
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	http_string_t *body_content = (http_string_t*)event->user_defined;
	assert(body_content != NULL);

	engine_work_t *engine_work = event->work;
	assert(engine_work != NULL);

	http_body_t *body = *(http_body_t**)event->predefined;
	if(!body)
	{
		http_debug(debug_http_detect, "null http body\n");
		http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
		return 0;
	}
	
	if(body->info.content_type.subtype != HTTP_BODY_CONTENT_SUBTYPE_X_WWW_FORM_URLENCODED || 
		body->info.content_type.maintype != HTTP_BODY_CONTENT_MAINTYPE_APPLICATION)
	{
		http_debug(debug_http_detect, "no need do body preprocessing\n");
		http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
		return 0;
	}

	if(http_body_merge((http_work_t)engine_work->predefined, body, body_content, 1))
	{
		http_debug(debug_http_detect, "merge http body failed\n");
		http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
		return -1;
	}
	
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 0;
}

int http_request_uri_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event)
{
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	http_debug(debug_http_detect, ">>>>>>>>>>>>>>>>>%s return 1\n", __FUNCTION__);
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 1;
}

int http_request_body_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event)
{
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	http_debug(debug_http_detect, ">>>>>>>>>>>>>>>>>%s return 1\n", __FUNCTION__);
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 1;
}
