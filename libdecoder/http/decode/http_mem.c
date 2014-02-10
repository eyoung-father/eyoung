#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "http.h"
#include "http_private.h"
#include "http_client_parser.h"
#include "http_server_parser.h"
#include "http_client_lex.h"
#include "http_server_lex.h"

extern int http_cmd_pair_id;

int http_mem_init(http_decoder_t *decoder)
{
	decoder->http_data_slab = http_zinit("http private data", sizeof(http_data_t));
	if(!decoder->http_data_slab)
	{
		http_debug(debug_http_mem, "init http_data_slab failed\n");
		return -1;
	}

	decoder->http_cmd_slab = http_zinit("http command", sizeof(http_cmd_t));
	if(!decoder->http_cmd_slab)
	{
		http_debug(debug_http_mem, "init http_cmd_slab failed\n");
		return -1;
	}
		
	/*init client side*/
	decoder->http_request_value_fslab = http_fzinit("http request value fslab", DEFAULT_VALUE_LENGTH);
	if(!decoder->http_request_value_fslab)
	{
		http_debug(debug_http_mem, "init http_request_value_fslab failed\n");
		return -1;
	}
	
	decoder->http_request_first_line_slab = http_zinit("http request first line slab", sizeof(http_request_first_line_t));
	if(!decoder->http_request_first_line_slab)
	{
		http_debug(debug_http_mem, "init http_request_first_line_slab failed\n");
		return -1;
	}
	
	decoder->http_request_header_slab = http_zinit("http request header", sizeof(http_request_header_t));
	if(!decoder->http_request_header_slab)
	{
		http_debug(debug_http_mem, "init http_request_header_slab failed\n");
		return -1;
	}
	
	decoder->http_request_string_part_slab = http_zinit("http request string part", sizeof(http_string_list_part_t));
	if(!decoder->http_request_string_part_slab)
	{
		http_debug(debug_http_mem, "init http_request_string_part_slab failed\n");
		return -1;
	}

	decoder->http_request_chunk_part_slab = http_zinit("http request chunk part", sizeof(http_chunk_body_part_t));
	if(!decoder->http_request_chunk_part_slab)
	{
		http_debug(debug_http_mem, "init http_request_chunk_part_slab failed\n");
		return -1;
	}

	decoder->http_request_body_slab = http_zinit("http request body", sizeof(http_body_t));
	if(!decoder->http_request_body_slab)
	{
		http_debug(debug_http_mem, "init http_request_body_slab failed\n");
		return -1;
	}

	decoder->http_request_slab = http_zinit("http request data", sizeof(http_request_t));
	if(!decoder->http_request_slab)
	{
		http_debug(debug_http_mem, "init http_request_slab failed\n");
		return -1;
	}
	
	/*init server side*/
	decoder->http_response_value_fslab = http_fzinit("http response value fslab", DEFAULT_VALUE_LENGTH);
	if(!decoder->http_response_value_fslab)
	{
		http_debug(debug_http_mem, "init http_response_value_fslab failed\n");
		return -1;
	}
	
	decoder->http_response_first_line_slab = http_zinit("http response first line slab", sizeof(http_response_first_line_t));
	if(!decoder->http_response_first_line_slab)
	{
		http_debug(debug_http_mem, "init http_response_first_line_slab failed\n");
		return -1;
	}
	
	decoder->http_response_header_slab = http_zinit("http response header", sizeof(http_response_header_t));
	if(!decoder->http_response_header_slab)
	{
		http_debug(debug_http_mem, "init http_response_header_slab failed\n");
		return -1;
	}
	
	decoder->http_response_string_part_slab = http_zinit("http response string part", sizeof(http_string_list_part_t));
	if(!decoder->http_response_string_part_slab)
	{
		http_debug(debug_http_mem, "init http_response_string_part_slab failed\n");
		return -1;
	}

	decoder->http_response_chunk_part_slab = http_zinit("http response chunk part", sizeof(http_chunk_body_part_t));
	if(!decoder->http_response_chunk_part_slab)
	{
		http_debug(debug_http_mem, "init http_response_chunk_part_slab failed\n");
		return -1;
	}

	decoder->http_response_body_slab = http_zinit("http response body", sizeof(http_body_t));
	if(!decoder->http_response_body_slab)
	{
		http_debug(debug_http_mem, "init http_response_body_slab failed\n");
		return -1;
	}

	decoder->http_response_slab = http_zinit("http response data", sizeof(http_response_t));
	if(!decoder->http_response_slab)
	{
		http_debug(debug_http_mem, "init http_response_slab failed\n");
		return -1;
	}

	return 0;
}

void http_mem_finit(http_decoder_t *decoder)
{
	http_zfinit(decoder->http_data_slab);
	http_zfinit(decoder->http_cmd_slab);

	http_fzfinit(decoder->http_request_value_fslab);
	http_zfinit(decoder->http_request_first_line_slab);
	http_zfinit(decoder->http_request_header_slab);
	http_zfinit(decoder->http_request_string_part_slab);
	http_zfinit(decoder->http_request_chunk_part_slab);
	http_zfinit(decoder->http_request_body_slab);
	http_zfinit(decoder->http_request_slab);

	http_fzfinit(decoder->http_response_value_fslab);
	http_zfinit(decoder->http_response_first_line_slab);
	http_zfinit(decoder->http_response_header_slab);
	http_zfinit(decoder->http_response_string_part_slab);
	http_zfinit(decoder->http_response_chunk_part_slab);
	http_zfinit(decoder->http_response_body_slab);
	http_zfinit(decoder->http_response_slab);
}

http_data_t* http_alloc_priv_data(http_decoder_t *decoder, int greedy)
{
	void *client_lexier = NULL;
	void *client_parser = NULL;
	void *server_lexier = NULL;
	void *server_parser = NULL;
	http_data_t *data = NULL;

	client_parser = (void*)http_client_pstate_new();
	if(!client_parser)
	{
		http_debug(debug_http_mem, "alloc client parser failed\n");
		goto failed;
	}

	if(http_client_lex_init(&client_lexier))
	{
		http_debug(debug_http_mem, "alloc client lexier failed\n");
		goto failed;
	}

	server_parser = (void*)http_server_pstate_new();
	if(!server_parser)
	{
		http_debug(debug_http_mem, "alloc server parser failed\n");
		goto failed;
	}

	if(http_server_lex_init(&server_lexier))
	{
		http_debug(debug_http_mem, "alloc server lexier failed\n");
		goto failed;
	}
	
	data = (http_data_t*)http_zalloc(decoder->http_data_slab);
	if(!data)
	{
		http_debug(debug_http_mem, "alloc private data failed\n");
		goto failed;
	}

	memset(data, 0, sizeof(*data));

	/*init client*/
	data->request_parser.parser = client_parser;
	data->request_parser.lexier = client_lexier;
	data->request_parser.greedy = greedy;
	STAILQ_INIT(&data->request_list);
	http_client_set_extra((void*)data, client_lexier);

	/*init server*/
	data->response_parser.parser = server_parser;
	data->response_parser.lexier = server_lexier;
	data->response_parser.greedy = greedy;
	STAILQ_INIT(&data->response_list);
	http_server_set_extra((void*)data, server_lexier);

	/*init session*/
	STAILQ_INIT(&data->cmd_list);
	return data;

failed:
	if(client_parser)
		http_client_pstate_delete((http_client_pstate*)client_parser);
	if(client_lexier)
		http_client_lex_destroy(client_lexier);
	if(server_parser)
		http_server_pstate_delete((http_server_pstate*)server_parser);
	if(server_lexier)
		http_server_lex_destroy(server_lexier);
	if(data)
		http_zfree(decoder->http_data_slab, data);
	return NULL;
}

void http_free_priv_data(http_decoder_t *decoder, http_data_t *data)
{
	if(!data)
		return;
	
	/*free client*/
	if(data->request_parser.parser)
		http_client_pstate_delete((http_client_pstate*)data->request_parser.parser);
	if(data->request_parser.lexier)
		http_client_lex_destroy(data->request_parser.lexier);
	if(data->request_parser.saved)
		http_free(data->request_parser.saved);
	http_client_free_request_list(decoder, &data->request_list);

	/*free server*/
	if(data->response_parser.parser)
		http_server_pstate_delete((http_server_pstate*)data->response_parser.parser);
	if(data->response_parser.lexier)
		http_server_lex_destroy(data->response_parser.lexier);
	if(data->response_parser.saved)
		http_free(data->response_parser.saved);
	http_server_free_response_list(decoder, &data->response_list);

	http_free_cmd_list(decoder, &data->cmd_list);

	/*free private data self*/
	http_zfree(decoder->http_data_slab, data);
}

http_cmd_t* http_alloc_cmd(http_decoder_t *decoder, http_request_t *request, http_response_t *response)
{
	http_cmd_t *cmd = (http_cmd_t*)http_zalloc(decoder->http_cmd_slab);
	if(!cmd)
	{
		http_debug(debug_http_mem, "alloc http cmd failed\n");
		return NULL;
	}
	memset(cmd, 0, sizeof(*cmd));
	cmd->request = request;
	cmd->response = response;
	return cmd;
}

void http_free_cmd(http_decoder_t *decoder, http_cmd_t *cmd)
{
	if(!cmd)
		return;
	http_client_free_request(decoder, cmd->request);
	http_server_free_response(decoder, cmd->response);
	http_zfree(decoder->http_cmd_slab, cmd);
}

void http_free_cmd_list(http_decoder_t *decoder, http_cmd_list_t *cmd_list)
{
	if(!cmd_list)
		return;
	
	http_cmd_t *cmd,*tmp;
	STAILQ_FOREACH_SAFE(cmd, cmd_list, next, tmp)
		http_free_cmd(decoder, cmd);
}

http_request_t* http_client_alloc_request(http_decoder_t *decoder,
										  http_request_first_line_t *first_line,
										  http_request_header_list_t *header_list,
										  http_body_t *body)
{
	http_request_t *request = http_zalloc(decoder->http_request_slab);
	if(!request)
	{
		http_debug(debug_http_mem, "alloc http request failed\n");
		return NULL;
	}

	memset(request, 0, sizeof(*request));

	request->first_line = first_line;

	STAILQ_INIT(&request->header_list);
	if(header_list)
		STAILQ_CONCAT(&request->header_list, header_list);
	request->body = body;
	
	return request;
}

void http_client_free_request(http_decoder_t *decoder, http_request_t*request)
{
	if(!request)
		return;
	
	http_client_free_first_line(decoder, request->first_line);
	http_client_free_header_list(decoder, &request->header_list);
	http_client_free_body(decoder, request->body);
}

void http_client_free_request_list(http_decoder_t *decoder, http_request_list_t *request_list)
{
	if(!request_list)
		return;
	
	http_request_t *req, *tmp;
	STAILQ_FOREACH_SAFE(req, request_list, next, tmp)
		http_client_free_request(decoder, req);
}

http_request_first_line_t *http_client_alloc_first_line(http_decoder_t *decoder,
										  http_request_method_t method,
										  http_string_t *uri,
										  http_version_t version)
{
	http_request_first_line_t *line = (http_request_first_line_t*)http_zalloc(decoder->http_request_first_line_slab);
	if(!line)
	{
		http_debug(debug_http_mem, "alloc http request first line failed\n");
		return NULL;
	}
	memset(line, 0, sizeof(*line));
	line->method = method;
	line->version = version;
	if(uri)
		line->uri = *uri;
	return line;
}

void http_client_free_first_line(http_decoder_t *decoder, http_request_first_line_t *first_line)
{
	if(!first_line)
		return;
	
	if(first_line->uri.buf)
		http_fzfree(decoder->http_request_value_fslab, first_line->uri.buf);
	http_zfree(decoder->http_request_first_line_slab, first_line);
}

http_request_header_t* http_client_alloc_header(http_decoder_t *decoder,
										  http_request_header_type_t type,
										  http_string_t *value)
{
	http_request_header_t *header = (http_request_header_t*)http_zalloc(decoder->http_request_header_slab);
	if(!header)
	{
		http_debug(debug_http_mem, "alloc http request header failed\n");
		return NULL;
	}
	
	memset(header, 0, sizeof(*header));
	header->type = type;
	if(value)
		header->value = *value;
	return header;
}

void http_client_free_header(http_decoder_t *decoder, http_request_header_t *header)
{
	if(!header)
		return;
	if(header->value.buf)
		http_fzfree(decoder->http_request_value_fslab, header->value.buf);
	http_zfree(decoder->http_request_header_slab, header);
}

void http_client_free_header_list(http_decoder_t *decoder, http_request_header_list_t *header_list)
{
	if(!header_list)
		return;
	
	http_request_header_t *header, *tmp;
	STAILQ_FOREACH_SAFE(header, header_list, next, tmp)
		http_client_free_header(decoder, header);
}

http_response_t* http_server_alloc_response(http_decoder_t *decoder,
										  http_response_first_line_t *first_line,
										  http_response_header_list_t *header_list,
										  http_body_t *body)
{
	http_response_t *response = http_zalloc(decoder->http_response_slab);
	if(!response)
	{
		http_debug(debug_http_mem, "alloc http response failed\n");
		return NULL;
	}

	memset(response, 0, sizeof(*response));

	response->first_line = first_line;

	STAILQ_INIT(&response->header_list);
	if(header_list)
		STAILQ_CONCAT(&response->header_list, header_list);
	response->body = body;
	
	return response;
}

void http_server_free_response(http_decoder_t *decoder, http_response_t*response)
{
	if(!response)
		return;
	
	http_server_free_first_line(decoder, response->first_line);
	http_server_free_header_list(decoder, &response->header_list);
	http_server_free_body(decoder, response->body);
}

void http_server_free_response_list(http_decoder_t *decoder, http_response_list_t *response_list)
{
	if(!response_list)
		return;
	
	http_response_t *req, *tmp;
	STAILQ_FOREACH_SAFE(req, response_list, next, tmp)
		http_server_free_response(decoder, req);
}

http_response_first_line_t *http_server_alloc_first_line(http_decoder_t *decoder,
										  http_version_t version,
										  http_response_code_t code,
										  http_string_t *message)
{
	http_response_first_line_t *line = (http_response_first_line_t*)http_zalloc(decoder->http_response_first_line_slab);
	if(!line)
	{
		http_debug(debug_http_mem, "alloc http response first line failed\n");
		return NULL;
	}
	memset(line, 0, sizeof(*line));
	line->version = version;
	line->code = code;
	if(message)
		line->message = *message;
	return line;
}

void http_server_free_first_line(http_decoder_t *decoder, http_response_first_line_t *first_line)
{
	if(!first_line)
		return;
	
	if(first_line->message.buf)
		http_fzfree(decoder->http_response_value_fslab, first_line->message.buf);
	http_zfree(decoder->http_response_first_line_slab, first_line);
}

http_response_header_t* http_server_alloc_header(http_decoder_t *decoder,
										  http_response_header_type_t type,
										  http_string_t *value)
{
	http_response_header_t *header = (http_response_header_t*)http_zalloc(decoder->http_response_header_slab);
	if(!header)
	{
		http_debug(debug_http_mem, "alloc http response header failed\n");
		return NULL;
	}
	
	memset(header, 0, sizeof(*header));
	header->type = type;
	if(value)
		header->value = *value;
	return header;
}

void http_server_free_header(http_decoder_t *decoder, http_response_header_t *header)
{
	if(!header)
		return;
	if(header->value.buf)
		http_fzfree(decoder->http_response_value_fslab, header->value.buf);
	http_zfree(decoder->http_response_header_slab, header);
}

void http_server_free_header_list(http_decoder_t *decoder, http_response_header_list_t *header_list)
{
	if(!header_list)
		return;
	
	http_response_header_t *header, *tmp;
	STAILQ_FOREACH_SAFE(header, header_list, next, tmp)
		http_server_free_header(decoder, header);
}

void http_free_string(http_decoder_t *decoder, http_string_t *string, int from_client)
{
	if(!string || !string->buf)
		return;
	if(from_client)
		http_fzfree(decoder->http_request_value_fslab, string->buf);
	else
		http_fzfree(decoder->http_response_value_fslab, string->buf);
	string->buf = NULL;
	string->len = 0;
}

char* http_alloc_string(http_decoder_t *decoder, const char *i_str, size_t i_len, http_string_t *o_str, int from_client)
{
	if(!o_str)
		return NULL;
	
	char *ret = NULL;
	if(from_client)
		ret = (char*)http_fzalloc(i_len+1, decoder->http_request_value_fslab);
	else
		ret = (char*)http_fzalloc(i_len+1, decoder->http_response_value_fslab);
	if(ret)
	{
		if(i_str)
			memcpy(ret, i_str, i_len);
		ret[i_len] = '\0';
		o_str->buf = ret;
		o_str->len = i_len;
	}
	return ret;
}

http_string_list_part_t* http_alloc_string_list_part(http_decoder_t *decoder, const http_string_t *src, int from_client)
{
	if(!src)
		return NULL;
	http_string_list_part_t *ret = NULL;
	if(from_client)
		ret = (http_string_list_part_t*)http_zalloc(decoder->http_request_string_part_slab);
	else
		ret = (http_string_list_part_t*)http_zalloc(decoder->http_response_string_part_slab);
	
	memset(ret, 0, sizeof(*ret));
	ret->string = *src;
	return ret;
}

void http_free_string_list_part(http_decoder_t *decoder, http_string_list_part_t *src, int from_client)
{
	if(!src)
		return;
	http_free_string(decoder, &src->string, from_client);
	if(from_client)
		http_zfree(decoder->http_request_string_part_slab, src);
	else
		http_zfree(decoder->http_response_string_part_slab, src);
}

void http_free_string_list(http_decoder_t *decoder, http_string_list_t *list_head, int from_client)
{
	if(!list_head)
		return;
	
	http_string_list_part_t *entry=NULL, *tmp=NULL;
	STAILQ_FOREACH_SAFE(entry, list_head, next, tmp)
		http_free_string_list_part(decoder, entry, from_client);
}

http_chunk_body_part_t* http_alloc_chunk_body_part(http_decoder_t *decoder, int from_client)
{
	http_chunk_body_part_t *ret = NULL;
	if(from_client)
		ret = (http_chunk_body_part_t*)http_zalloc(decoder->http_request_chunk_part_slab);
	else
		ret = (http_chunk_body_part_t*)http_zalloc(decoder->http_response_chunk_part_slab);
	
	if(ret)
	{
		memset(ret, 0, sizeof(*ret));
		STAILQ_INIT(&ret->chunk_value);
	}
	return ret;
}

void http_free_chunk_body_part(http_decoder_t *decoder, http_chunk_body_part_t *src, int from_client)
{
	if(!src)
		return;
	http_free_string(decoder, &src->chunk_header.chunk_extension, from_client);
	http_free_string_list(decoder, &src->chunk_value, from_client);
	if(from_client)
		http_zfree(decoder->http_request_chunk_part_slab, src);
	else
		http_zfree(decoder->http_response_chunk_part_slab, src);
}

void http_free_chunk_body_list(http_decoder_t *decoder, http_chunk_body_list_t *list, int from_client)
{
	if(!list)
		return;
	
	http_chunk_body_part_t *part=NULL, *tmp=NULL;
	STAILQ_FOREACH_SAFE(part, list, next, tmp)
		http_free_chunk_body_part(decoder, part, from_client);
}

void http_free_chunk_body(http_decoder_t *decoder, http_chunk_body_t *body, int from_client)
{
	if(!body)
		return;
	
	http_free_chunk_body_list(decoder, &body->chunk_list, from_client);
	http_free_string_list(decoder, &body->chunk_tailer, from_client);
}

http_body_t* http_alloc_body(http_decoder_t *decoder, int from_client)
{
	http_body_t *ret = NULL;
	if(from_client)
		ret = (http_body_t*)http_zalloc(decoder->http_request_body_slab);
	else
		ret = (http_body_t*)http_zalloc(decoder->http_response_body_slab);
	
	if(ret)
	{
		memset(ret, 0, sizeof(*ret));
		STAILQ_INIT(&ret->normal_body);
		STAILQ_INIT(&ret->chunk_body.chunk_list);
		STAILQ_INIT(&ret->chunk_body.chunk_tailer);
		ret->info.content_type.maintype = HTTP_BODY_CONTENT_MAINTYPE_UNKOWN;
		ret->info.content_type.subtype = HTTP_BODY_CONTENT_SUBTYPE_UNKOWN;
		ret->info.content_encoding = HTTP_BODY_CONTENT_ENCODING_UNKOWN;
		ret->info.transfer_encoding = HTTP_BODY_TRANSFER_ENCODING_UNKOWN;
		ret->info.content_language = HTTP_BODY_CONTent_LANGUAGE_UNKOWN;
		ret->info.content_charset = HTTP_BODY_CONTENT_CHARSET_UNKOWN;
	}
	return ret;
}

void http_free_body(http_decoder_t *decoder, http_body_t *body, int from_client)
{
	if(!body)
		return;
	http_free_string_list(decoder, &body->normal_body, from_client);
	http_free_string_list(decoder, &body->chunk_body.chunk_tailer, from_client);
	http_free_chunk_body_list(decoder, &body->chunk_body.chunk_list, from_client);
	if(from_client)
		http_zfree(decoder->http_request_body_slab, body);
	else
		http_zfree(decoder->http_response_body_slab, body);
}
