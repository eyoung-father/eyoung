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
	if(body->info.transfer_encoding != HTTP_BODY_TRANSFER_ENCODING_CHUNKED)
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
