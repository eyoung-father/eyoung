#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "pop3.h"
#include "pop3_type.h"
#include "pop3_util.h"
#include "pop3_mem.h"
#include "pop3_client_parser.h"
#include "pop3_server_parser.h"
#include "pop3_client_lex.h"
#include "pop3_server_lex.h"

extern int pop3_cmd_pair_id;

void pop3_free_request(pop3_decoder_t *decoder, pop3_request_t *req)
{
	if(!req)
		return;
	switch(req->req_code)
	{
		case POP3_COMMAND_USER:
		{
			if(req->user.user)
				pop3_free(req->user.user);
			break;
		}
		case POP3_COMMAND_PASS:
		{
			if(req->pass.pass)
				pop3_free(req->pass.pass);
			break;
		}
		case POP3_COMMAND_APOP:
		{
			if(req->apop.pass)
				pop3_free(req->apop.pass);
			break;
		}
		case POP3_COMMAND_UNKNOWN:
		{
			if(req->unknown.name)
				pop3_free(req->unknown.name);
			break;
		}
		default:
		{
			break;
		}
	}
	pop3_zfree(decoder->pop3_request_slab, req);
}

void pop3_free_response_content(pop3_decoder_t *decoder, pop3_res_content_t *head)
{
	pop3_line_t *line = NULL;
	pop3_line_t *next_line = NULL;

	if(!head)
		return;
		
	STAILQ_FOREACH_SAFE(line, head, next, next_line)
	{
		if(line->line)
			pop3_free(line->line);
		pop3_zfree(decoder->pop3_res_line_slab, line);
	}
}

void pop3_free_response(pop3_decoder_t *decoder, pop3_response_t *res)
{
	if(!res)
		return;
	if(res->msg)
		pop3_free(res->msg);
	pop3_free_response_content(decoder, &res->content);
	pop3_zfree(decoder->pop3_response_slab, res);
}

static void pop3_free_cmd(pop3_decoder_t *decoder, pop3_cmd_t *cmd)
{
	if(!cmd)
		return;
	if(cmd->req)
		pop3_free_request(decoder, cmd->req);
	if(cmd->res)
		pop3_free_response(decoder, cmd->res);
	pop3_zfree(decoder->pop3_cmd_slab, cmd);
}

int pop3_mem_init(pop3_decoder_t *decoder)
{
	decoder->pop3_data_slab = pop3_zinit("pop3 private data", sizeof(pop3_data_t));
	if(!decoder->pop3_data_slab)
	{
		pop3_debug(debug_pop3_mem, "init pop3_data_slab failed\n");
		return -1;
	}

	decoder->pop3_request_slab = pop3_zinit("pop3 request data", sizeof(pop3_request_t));
	if(!decoder->pop3_request_slab)
	{
		pop3_debug(debug_pop3_mem, "init pop3_request_slab failed\n");
		return -1;
	}

	decoder->pop3_response_slab = pop3_zinit("pop3 response data", sizeof(pop3_response_t));
	if(!decoder->pop3_response_slab)
	{
		pop3_debug(debug_pop3_mem, "init pop3_response_slab failed\n");
		return -1;
	}

	decoder->pop3_cmd_slab = pop3_zinit("pop3 command", sizeof(pop3_cmd_t));
	if(!decoder->pop3_cmd_slab)
	{
		pop3_debug(debug_pop3_mem, "init pop3_cmd_slab failed\n");
		return -1;
	}

	decoder->pop3_req_arg_slab = pop3_zinit("pop3 request argument", sizeof(pop3_req_arg_t));
	if(!decoder->pop3_req_arg_slab)
	{
		pop3_debug(debug_pop3_mem, "init pop3_req_arg_slab failed\n");
		return -1;
	}

	decoder->pop3_res_line_slab = pop3_zinit("pop3 response content line", sizeof(pop3_line_t));
	if(!decoder->pop3_res_line_slab)
	{
		pop3_debug(debug_pop3_mem, "init pop3_res_line_slab failed\n");
		return -1;
	}

	return 0;
}

void pop3_mem_finit(pop3_decoder_t *decoder)
{
	pop3_zfinit(decoder->pop3_data_slab);
	pop3_zfinit(decoder->pop3_request_slab);
	pop3_zfinit(decoder->pop3_response_slab);
	pop3_zfinit(decoder->pop3_cmd_slab);
	pop3_zfinit(decoder->pop3_req_arg_slab);
	pop3_zfinit(decoder->pop3_res_line_slab);
}

pop3_data_t* pop3_alloc_priv_data(pop3_decoder_t *decoder, int greedy)
{
	void *client_lexier = NULL;
	void *client_parser = NULL;
	void *server_lexier = NULL;
	void *server_parser = NULL;
	pop3_data_t *data = NULL;

	client_parser = (void*)pop3_client_pstate_new();
	if(!client_parser)
	{
		pop3_debug(debug_pop3_mem, "alloc client parser failed\n");
		goto failed;
	}

	if(pop3_client_lex_init(&client_lexier))
	{
		pop3_debug(debug_pop3_mem, "alloc client lexier failed\n");
		goto failed;
	}

	server_parser = (void*)pop3_server_pstate_new();
	if(!server_parser)
	{
		pop3_debug(debug_pop3_mem, "alloc server parser failed\n");
		goto failed;
	}

	if(pop3_server_lex_init(&server_lexier))
	{
		pop3_debug(debug_pop3_mem, "alloc server lexier failed\n");
		goto failed;
	}
	
	data = (pop3_data_t*)pop3_zalloc(decoder->pop3_data_slab);
	if(!data)
	{
		pop3_debug(debug_pop3_mem, "alloc private data failed\n");
		goto failed;
	}

	memset(data, 0, sizeof(*data));

	/*init client*/
	data->request_parser.parser = client_parser;
	data->request_parser.lexier = client_lexier;
	data->request_parser.greedy = greedy;
	STAILQ_INIT(&data->request_list);
	pop3_client_set_extra((void*)data, client_lexier);

	/*init server*/
	data->response_parser.parser = server_parser;
	data->response_parser.lexier = server_lexier;
	data->response_parser.greedy = greedy;
	STAILQ_INIT(&data->response_list);
	pop3_server_set_extra((void*)data, server_lexier);

	/*init session*/
	STAILQ_INIT(&data->cmd_list);
	return data;

failed:
	if(client_parser)
		pop3_client_pstate_delete((pop3_client_pstate*)client_parser);
	if(client_lexier)
		pop3_client_lex_destroy(client_lexier);
	if(server_parser)
		pop3_server_pstate_delete((pop3_server_pstate*)server_parser);
	if(server_lexier)
		pop3_server_lex_destroy(server_lexier);
	if(data)
		pop3_zfree(decoder->pop3_data_slab, data);
	return NULL;
}

void pop3_free_priv_data(pop3_decoder_t *decoder, pop3_data_t *data)
{
	if(!data)
		return;
	
	/*free client*/
	if(data->request_parser.parser)
		pop3_client_pstate_delete((pop3_client_pstate*)data->request_parser.parser);
	if(data->request_parser.lexier)
		pop3_client_lex_destroy(data->request_parser.lexier);
	if(data->request_parser.saved)
		pop3_free(data->request_parser.saved);
	pop3_request_t *req = NULL, *next_req = NULL;
	STAILQ_FOREACH_SAFE(req, &data->request_list, next, next_req)
	{
		pop3_free_request(decoder, req);
	}

	/*free server*/
	if(data->response_parser.parser)
		pop3_server_pstate_delete((pop3_server_pstate*)data->response_parser.parser);
	if(data->response_parser.lexier)
		pop3_server_lex_destroy(data->response_parser.lexier);
	if(data->response_parser.saved)
		pop3_free(data->response_parser.saved);
	pop3_response_t *res = NULL, *next_res = NULL;
	STAILQ_FOREACH_SAFE(res, &data->response_list, next, next_res)
	{
		pop3_free_response(decoder, res);
	}

	/*free session*/
	pop3_cmd_t *cmd = NULL, *next_cmd = NULL;
	STAILQ_FOREACH_SAFE(cmd, &data->cmd_list, next, next_cmd)
	{
		pop3_free_cmd(decoder, cmd);
	}

	/*free private data self*/
	pop3_zfree(decoder->pop3_data_slab, data);
}

pop3_response_t* pop3_alloc_response(pop3_decoder_t *decoder, int res_code, char *msg, int msg_len, pop3_res_content_t *content)
{
	pop3_response_t *res = (pop3_response_t*)pop3_zalloc(decoder->pop3_response_slab);
	if(!res)
	{
		pop3_debug(debug_pop3_mem, "alloc pop3 response failed\n");
		goto failed;
	}
	memset(res, 0, sizeof(*res));
	STAILQ_INIT(&res->content);
	res->res_code = res_code;
	res->len = msg_len;
	res->msg = msg;
	if(content)
		STAILQ_CONCAT(&res->content, content);
	pop3_debug(debug_pop3_mem, "alloc pop3 response: res_code:%s, msg: %s\n", pop3_res_type_name(res_code), msg?msg:"(NULL)");
	return res;

failed:
	if(res)
		pop3_zfree(decoder->pop3_response_slab, res);
	return NULL;
}

pop3_line_t* pop3_alloc_response_line(pop3_decoder_t *decoder, char *data, int data_len)
{
	pop3_line_t* line = (pop3_line_t*)pop3_zalloc(decoder->pop3_res_line_slab);
	if(!line)
	{
		pop3_debug(debug_pop3_mem, "alloc pop3 response line failed\n");
		goto failed;
	}
	memset(line, 0, sizeof(*line));
	line->line = data;
	line->line_len = data_len;
	pop3_debug(debug_pop3_mem, "alloc pop3 response line: data: %s\n", data?data:"(NULL)");
	return line;

failed:
	if(line)
		pop3_zfree(decoder->pop3_res_line_slab, line);
	return NULL;
}

pop3_request_t* pop3_alloc_request(pop3_decoder_t *decoder, int req_code, ...)
{
	va_list ap;
	va_start(ap, req_code);
	pop3_request_t *req = (pop3_request_t*)pop3_zalloc(decoder->pop3_request_slab);
	if(!req)
	{
		pop3_debug(debug_pop3_mem, "alloc pop3 request failed\n");
		goto failed;
	}
	memset(req, 0, sizeof(*req));
	req->req_code = req_code;
	switch(req_code)
	{
		case POP3_COMMAND_USER:
		{
			req->user.user = va_arg(ap, char*);
			req->user.len = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_PASS:
		{
			req->pass.pass = va_arg(ap, char*);
			req->pass.len = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_APOP:
		{
			req->apop.pass = va_arg(ap, char*);
			req->apop.len = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_TOP:
		{
			req->top.mail = va_arg(ap, int);
			req->top.lines = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_UIDL:
		{
			req->uidl.mail = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_LIST:
		{
			req->list.mail = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_RETR:
		{
			req->retr.mail = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_DELE:
		{
			req->dele.mail = va_arg(ap, int);
			break;
		}
		case POP3_COMMAND_RSET:
		{
			req->rset = req_code;
			break;
		}
		case POP3_COMMAND_QUIT:
		{
			req->quit = req_code;
			break;
		}
		case POP3_COMMAND_NOOP:
		{
			req->noop = req_code;
			break;
		}
		case POP3_COMMAND_STAT:
		{
			req->stat = req_code;
			break;
		}
		case POP3_COMMAND_UNKNOWN:
		{
			req->unknown.name = va_arg(ap, char*);
			req->unknown.len = va_arg(ap, int);
			break;
		}
		default:
		{
			pop3_debug(debug_pop3_mem, "bad request code %d\n", req_code);
			goto failed;
		}
	}
	va_end(ap);
	pop3_debug(debug_pop3_mem, "alloc pop3 request, req_code: %s\n", pop3_req_type_name(req_code));
	return req;

failed:
	if(req)
		pop3_zfree(decoder->pop3_request_slab, req);
	va_end(ap);
	return NULL;
}

pop3_req_arg_t* pop3_alloc_req_arg(pop3_decoder_t *decoder, char *data, int data_len)
{
	if(!data || data_len<=0)
		return NULL;

	pop3_req_arg_t *arg = NULL;
	char *msg = NULL;

	arg = (pop3_req_arg_t*)pop3_zalloc(decoder->pop3_req_arg_slab);
	if(!arg)
	{
		pop3_debug(debug_pop3_mem, "alloc arg failed\n");
		goto failed;
	}
	memset(arg, 0, sizeof(*arg));

	msg = pop3_malloc(data_len+1);
	if(!msg)
	{
		pop3_debug(debug_pop3_mem, "alloc arg buffer failed, len:%d\n", data_len);
		goto failed;
	}
	memcpy(msg, data, data_len);
	msg[data_len] = '\0';

	arg->arg = msg;
	arg->len = data_len;
	pop3_debug(debug_pop3_mem, "alloc pop3 request arg, data: %s\n", data?data:"(NULL)");
	return arg;

failed:
	if(msg)
		pop3_free(msg);
	if(arg)
		pop3_zfree(decoder->pop3_req_arg_slab, arg);
	return NULL;
}

void pop3_free_req_arg_list(pop3_decoder_t *decoder, pop3_req_arg_list_t *head)
{
	if(!head)
		return;
	
	pop3_req_arg_t *arg = NULL, *next_arg = NULL;
	STAILQ_FOREACH_SAFE(arg, head, next, next_arg)
	{
		if(arg->arg)
			pop3_free(arg->arg);
		pop3_zfree(decoder->pop3_req_arg_slab, arg);
	}
}

pop3_cmd_t* pop3_alloc_cmd(pop3_decoder_t *decoder, pop3_request_t *req, pop3_response_t *res)
{
	pop3_cmd_t *cmd = pop3_zalloc(decoder->pop3_cmd_slab);
	if(!cmd)
	{
		pop3_debug(debug_pop3_mem, "alloc cmd failed\n");
		return NULL;
	}

	cmd->req = req;
	cmd->res = res;
	pop3_debug(debug_pop3_mem, "alloc pop3 command pair(req: %s, res: %s)\n", pop3_req_type_name(req?req->req_code:-1), pop3_res_type_name(res?res->res_code:-1));
	return cmd;
}

void pop3_free_cmd_list(pop3_decoder_t *decoder, pop3_cmd_list_t *head)
{
	if(!head)
		return;
	pop3_cmd_t *cmd = NULL, *next_cmd = NULL;
	STAILQ_FOREACH_SAFE(cmd, head, next, next_cmd)
	{
		if(cmd->req)
			pop3_free_request(decoder, cmd->req);
		if(cmd->res)
			pop3_free_response(decoder, cmd->res);
		pop3_zfree(decoder->pop3_cmd_slab, cmd);
	}
}

int pop3_add_command(pop3_decoder_t *decoder, pop3_data_t *priv_data)
{
	pop3_response_t *res = STAILQ_FIRST(&priv_data->response_list);
	pop3_request_t *req = STAILQ_FIRST(&priv_data->request_list);
	assert(res != NULL);

	pop3_cmd_t *cmd = pop3_alloc_cmd(decoder, req, res);
	if(!cmd)
	{
		pop3_debug(debug_pop3_mem, "failed to alloc command\n");
		return 1;
	}

	if(res)
		STAILQ_REMOVE_HEAD(&priv_data->response_list, next);
	if(req)
		STAILQ_REMOVE_HEAD(&priv_data->request_list, next);
	STAILQ_INSERT_TAIL(&priv_data->cmd_list, cmd, next);

	if(pop3_element_detect(priv_data, "cmd_pair", pop3_cmd_pair_id, &cmd) < 0)
	{
		pop3_debug(debug_pop3_detect, "find attack\n");
		return -1;
	}
	return 0;
}
