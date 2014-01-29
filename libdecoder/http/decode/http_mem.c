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
	
	decoder->http_request_body_part_slab = http_zinit("http request body part", sizeof(http_request_body_part_t));
	if(!decoder->http_request_body_part_slab)
	{
		http_debug(debug_http_mem, "init http_request_body_part_slab failed\n");
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
	
	decoder->http_response_body_part_slab = http_zinit("http response body part", sizeof(http_response_body_part_t));
	if(!decoder->http_response_body_part_slab)
	{
		http_debug(debug_http_mem, "init http_response_body_part_slab failed\n");
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
	http_zfinit(decoder->http_request_body_part_slab);
	http_zfinit(decoder->http_request_slab);

	http_fzfinit(decoder->http_response_value_fslab);
	http_zfinit(decoder->http_response_first_line_slab);
	http_zfinit(decoder->http_response_header_slab);
	http_zfinit(decoder->http_response_body_part_slab);
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
										  http_request_body_t *body)
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
	
	STAILQ_INIT(&request->body);
	if(body)
		STAILQ_CONCAT(&request->body, body);
	
	return request;
}

void http_client_free_request(http_decoder_t *decoder, http_request_t*request)
{
	if(!request)
		return;
	
	http_client_free_first_line(decoder, request->first_line);
	http_client_free_header_list(decoder, &request->header_list);
	http_client_free_body(decoder, &request->body);
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
										  http_request_string_t *uri,
										  http_request_version_t version)
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
										  http_request_string_t *value)
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

http_request_body_part_t* http_client_alloc_body_part(http_decoder_t *decoder,
										  http_request_string_t *value)
{
	http_request_body_part_t *part = (http_request_body_part_t*)http_zalloc(decoder->http_request_body_part_slab);
	if(!part)
	{
		http_debug(debug_http_mem, "alloc http request body part failed\n");
		return NULL;
	}

	memset(part, 0, sizeof(*part));
	if(value)
		part->value = *value;
	return part;
}

void http_client_free_body_part(http_decoder_t *decoder, http_request_body_part_t *part)
{
	if(!part)
		return;
	if(part->value.buf)
		http_fzfree(decoder->http_request_value_fslab, part->value.buf);
	http_zfree(decoder->http_request_body_part_slab, part);
}

void http_client_free_body(http_decoder_t *decoder, http_request_body_t *body)
{
	if(!body)
		return;
	
	http_request_body_part_t *part, *tmp;
	STAILQ_FOREACH_SAFE(part, body, next, tmp)
		http_client_free_body_part(decoder, part);
}

http_response_t* http_server_alloc_response(http_decoder_t *decoder,
										  http_response_first_line_t *first_line,
										  http_response_header_list_t *header_list,
										  http_response_body_t *body)
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
	
	STAILQ_INIT(&response->body);
	if(body)
		STAILQ_CONCAT(&response->body, body);
	
	return response;
}

void http_server_free_response(http_decoder_t *decoder, http_response_t*response)
{
	if(!response)
		return;
	
	http_server_free_first_line(decoder, response->first_line);
	http_server_free_header_list(decoder, &response->header_list);
	http_server_free_body(decoder, &response->body);
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
										  http_response_version_t version,
										  http_response_code_t code,
										  http_response_string_t *message)
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
										  http_response_string_t *value)
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

http_response_body_part_t* http_server_alloc_body_part(http_decoder_t *decoder,
										  http_response_string_t *value)
{
	http_response_body_part_t *part = (http_response_body_part_t*)http_zalloc(decoder->http_response_body_part_slab);
	if(!part)
	{
		http_debug(debug_http_mem, "alloc http response body part failed\n");
		return NULL;
	}

	memset(part, 0, sizeof(*part));
	if(value)
		part->value = *value;
	return part;
}

void http_server_free_body_part(http_decoder_t *decoder, http_response_body_part_t *part)
{
	if(!part)
		return;
	if(part->value.buf)
		http_fzfree(decoder->http_response_value_fslab, part->value.buf);
	http_zfree(decoder->http_response_body_part_slab, part);
}

void http_server_free_body(http_decoder_t *decoder, http_response_body_t *body)
{
	if(!body)
		return;
	
	http_response_body_part_t *part, *tmp;
	STAILQ_FOREACH_SAFE(part, body, next, tmp)
		http_server_free_body_part(decoder, part);
}
