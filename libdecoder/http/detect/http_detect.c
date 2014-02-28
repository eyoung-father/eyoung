#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "http.h"
#include "http_detect.h"
#include "http_private.h"

static void http_free_parameter_list(http_decoder_t *decoder, parameter_list_t *p_list, int from_client)
{
	parameter_t *pm=NULL, *tmp=NULL;
	STAILQ_FOREACH_SAFE(pm, p_list, next, tmp)
	{
		http_free_string(decoder, &pm->name, from_client);
		http_free_string(decoder, &pm->value, from_client);
		http_free(pm);
	}
}

static int html_parse_urlencoded_parameter(http_decoder_t *decoder, http_string_t *raw_data, parameter_list_t *list)
{
	/*TODO*/
	return 0;
}

static int http_body_merge(http_decoder_t *decoder, http_body_t *body, http_string_t *body_string, int from_client)
{
	assert(body_string != NULL);
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
	request_data_t *req_data = NULL;
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	req_data = (request_data_t*)http_malloc(sizeof(request_data_t));
	if(!req_data)
	{
		http_debug(debug_http_detect, ">>>>>>>>>>>>>>>>>%s return 1\n", __FUNCTION__);
		http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
		event->user_defined = req_data;
		return 1;
	}
	memset(req_data, 0, sizeof(request_data_t));
	STAILQ_INIT(&req_data->parameter_list);
	event->user_defined = req_data;
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
		request_data_t *req_data = (request_data_t*)event->user_defined;
		
		http_free_string(http_decoder, &req_data->raw_data, 1);
		http_free_parameter_list(http_decoder, &req_data->parameter_list, 1);
		http_free(req_data);
		event->user_defined = NULL;
	}
	http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
	return 0;
}

int http_request_body_preprocessor(engine_work_event_t *event)
{
	http_debug(debug_http_detect, "\n=============ENTER %s=============\n", __FUNCTION__);
	request_data_t *req_data = (request_data_t*)event->user_defined;

	engine_work_t *engine_work = event->work;
	assert(engine_work != NULL);

	http_data_t *data = (http_data_t*)engine_work->predefined;
	assert(data != NULL);
	
	http_decoder_t *decoder = (http_decoder_t*)data->decoder;
	assert(decoder != NULL);

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

	if(http_body_merge(decoder, body, &req_data->raw_data, 1))
	{
		http_debug(debug_http_detect, "merge http body failed\n");
		http_debug(debug_http_detect, "=============EXIT %s=============\n", __FUNCTION__);
		return -1;
	}
	
	if(html_parse_urlencoded_parameter(decoder, &req_data->raw_data, &req_data->parameter_list))
	{
		http_debug(debug_http_detect, "parse url-encoded parameter list failed\n");
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
