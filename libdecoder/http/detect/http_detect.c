#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "http.h"
#include "http_detect.h"
#include "http_private.h"

#define ENTER										\
	do												\
	{												\
		http_debug(debug_http_detect,				\
		"\n=============ENTER %s=============\n",	\
		__FUNCTION__);								\
	}while(0)

#define LEAVE(r)									\
	do												\
	{												\
		http_debug(debug_http_detect,				\
		"=============EXIT %s(%d)=============\n",	\
		__FUNCTION__, r);							\
		return r;									\
	}while(0)
static void free_parameter_list(http_decoder_t *decoder, parameter_list_t *p_list)
{
	parameter_t *pm=NULL, *tmp=NULL;
	STAILQ_FOREACH_SAFE(pm, p_list, next, tmp)
	{
		http_client_free_string(decoder, &pm->name);
		http_client_free_string(decoder, &pm->value);
		http_free(pm);
	}
	STAILQ_INIT(p_list);
}

static void print_parameter_list(parameter_list_t *list)
{
	if(!debug_http_detect)
		return;
	
	parameter_t *p = NULL;
	int n=0;
	STAILQ_FOREACH(p, list, next)
	{
		http_debug(1, "\tparam[%d], name: %s, value: %s\n", n, p->name.buf, p->value.buf);
		n++;
	}
}

static parameter_t *parse_urlencoded_parameter(http_decoder_t *decoder, http_string_t *raw_data)
{
	ENTER;
	http_string_t name={NULL, 0};
	http_string_t value={NULL, 0};
	char *begin = raw_data->buf;
	char *scan = begin;
	char *tail = scan + raw_data->len;
	parameter_t *ret = NULL;

	while(*scan != '=' && scan < tail)
		scan++;
	
	if(!http_client_alloc_string(decoder, begin, scan-begin, &name))
	{
		http_debug(debug_http_detect, "alloc name string failed\n");
		goto failed;
	}

	if(scan<tail)
		scan++;
	
	if(!http_client_alloc_string(decoder, scan, tail-scan, &value))
	{
		http_debug(debug_http_detect, "alloc value string failed\n");
		goto failed;
	}
	http_string_decode(&value);

	ret = (parameter_t*)http_malloc(sizeof(parameter_t));
	if(!ret)
	{
		http_debug(debug_http_detect, "alloc return parameter failed\n");
		goto failed;
	}
	memset(ret, 0, sizeof(parameter_t));
	ret->name = name;
	ret->value = value;
	return ret;

failed:
	http_client_free_string(decoder, &name);
	http_client_free_string(decoder, &value);
	return NULL;
}

static int parse_urlencoded_parameter_list(http_decoder_t *decoder, http_string_t *raw_data, parameter_list_t *list)
{
	ENTER;
	char *scan = raw_data->buf;
	char *tail = scan + raw_data->len;
	char *ptr  = scan;
	int n=0;
	parameter_t *parameter=NULL;
	
	while(*ptr == '&' && ptr<tail)
		ptr++;
	
	for(scan=ptr; scan<tail; scan++)
	{
		if(*scan == '&')
		{
			size_t len = scan - ptr;
			http_string_t s = {ptr, len};
			assert(len>0);
			parameter = parse_urlencoded_parameter(decoder, &s);
			if(!parameter)
			{
				http_debug(debug_http_detect, "parse parameter %d failed\n", n);
				goto failed;
			}
			n++;
			STAILQ_INSERT_TAIL(list, parameter, next);
			ptr = scan;
			while(*ptr == '&' && ptr<tail)
				ptr++;
			scan = ptr - 1;
		}
		continue;
	}

	if(ptr<tail)
	{
		http_string_t s = {ptr, tail-ptr};

		parameter = parse_urlencoded_parameter(decoder, &s);
		if(!parameter)
		{
			http_debug(debug_http_detect, "parse parameter %d failed\n", n);
			goto failed;
		}
		n++;
		STAILQ_INSERT_TAIL(list, parameter, next);
	}
	http_debug(debug_http_detect, "find %d parameters\n", n);
	print_parameter_list(list);
	LEAVE(0);

failed:
	free_parameter_list(decoder, list);
	LEAVE(-1);
}

static int body_merge(http_decoder_t *decoder, http_body_t *body, http_string_t *body_string, int from_client)
{
	ENTER;
	assert(body_string != NULL);
	char *wt = NULL, *o_buf=NULL;
	size_t o_len = 0;
	
	if(!body)
	{
		http_debug(debug_http_detect, "%s: null body parameter\n", __FUNCTION__);
		LEAVE(-1);
	}

	o_len = body->info.body_size;
	o_buf = http_alloc_string(decoder, NULL, o_len, body_string, from_client);
	if(!o_buf)
	{
		http_debug(debug_http_detect, "%s: failed to alloc merged body space\n", __FUNCTION__);
		LEAVE(-1);
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
	LEAVE(0);
}

int http_request_uri_preprocessor(engine_work_event_t *event)
{
	ENTER;
	LEAVE(0);
}

int http_request_body_init(engine_work_event_t *event)
{
	ENTER;
	request_data_t *req_data = NULL;
	req_data = (request_data_t*)http_malloc(sizeof(request_data_t));
	if(!req_data)
	{
		event->user_defined = req_data;
		LEAVE(1);
	}
	memset(req_data, 0, sizeof(request_data_t));
	STAILQ_INIT(&req_data->parameter_list);
	event->user_defined = req_data;
	LEAVE(0);
}

int http_request_body_finit(engine_work_event_t *event)
{
	ENTER;
	if(event->user_defined)
	{
		engine_work_t *engine_work = event->work;
		http_data_t *http_data = (http_data_t*)engine_work->predefined;
		http_decoder_t *http_decoder = (http_decoder_t*)http_data->decoder;
		request_data_t *req_data = (request_data_t*)event->user_defined;
		
		http_client_free_string(http_decoder, &req_data->raw_data);
		free_parameter_list(http_decoder, &req_data->parameter_list);
		http_free(req_data);
		event->user_defined = NULL;
	}
	LEAVE(0);
}

int http_request_body_preprocessor(engine_work_event_t *event)
{
	ENTER;
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
		LEAVE(0);
	}

	if(body->info.content_type.subtype != HTTP_BODY_CONTENT_SUBTYPE_X_WWW_FORM_URLENCODED || 
		body->info.content_type.maintype != HTTP_BODY_CONTENT_MAINTYPE_APPLICATION)
	{
		http_debug(debug_http_detect, "no need do body preprocessing\n");
		LEAVE(0);
	}

	if(body_merge(decoder, body, &req_data->raw_data, 1))
	{
		http_debug(debug_http_detect, "merge http body failed\n");
		LEAVE(-1);
	}
	
	if(parse_urlencoded_parameter_list(decoder, &req_data->raw_data, &req_data->parameter_list))
	{
		http_debug(debug_http_detect, "parse url-encoded parameter list failed\n");
		LEAVE(-1);
	}
	LEAVE(0);
}

int http_request_uri_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event)
{
	ENTER;
	LEAVE(1);
}

int http_request_body_xss_check(engine_work_t *engine_work, engine_work_event_t *work_event)
{
	ENTER;
	LEAVE(1);
}
