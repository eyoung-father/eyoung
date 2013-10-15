#include <stdlib.h>
#include <assert.h>

#include "pop3_type.h"
#include "pop3_detect.h"
#include "pop3_util.h"
#include "pop3_mem.h"

int pop3_do_weak_password_check(struct pop3_data *priv_data)
{
	pop3_debug(debug_pop3_ips, "do weak password check\n");
	return 0;
}

int pop3_do_brute_force_check(struct pop3_data *priv_data)
{
	pop3_debug(debug_pop3_ips, "do auth brute force check\n");
	return 0;
}

int pop3_do_rule_detect(struct pop3_data *priv_data, const char* element_name, void *element_data)
{
	pop3_debug(debug_pop3_ips, "do signature check for %s\n", element_name);
	return 0;
}

int pop3_add_command(pop3_data_t *priv_data)
{
	pop3_response_t *res = STAILQ_FIRST(&priv_data->response_list);
	pop3_request_t *req = STAILQ_FIRST(&priv_data->request_list);
	assert(res != NULL);

	pop3_cmd_t *cmd = pop3_alloc_cmd(req, res);
	if(!cmd)
	{
		pop3_debug(debug_pop3_server, "failed to alloc command\n");
		return 1;
	}

	if(res)
		STAILQ_REMOVE_HEAD(&priv_data->response_list, next);
	if(req)
	{
		STAILQ_REMOVE_HEAD(&priv_data->request_list, next);
		pop3_debug(debug_pop3_server, "<==========dequeue pop3 request\n");
	}
	STAILQ_INSERT_TAIL(&priv_data->cmd_list, cmd, next);
	return 0;
}

int pop3_state_check(pop3_data_t *priv_data)
{
	static int check_table[POP3_STATE_MAX][POP3_COMMAND_MAX] = 
	{
				/*USER, PASS, APOP, LIST, RETR, DELE, UIDL, TOP, STAT, QUIT, NOOP, RSET, UNKNOWN*/
	/*INIT*/	{  0,    0,    0,    0,    0,    0,    0,    0,   0,    0,    0,    0,     1},
	/*AUTH*/	{  1,    1,    1,    0,    0,    0,    0,    0,   0,    1,    0,    0,     1},
	/*TRAN*/	{  0,    0,    0,    1,    1,    1,    1,    1,   1,    1,    1,    1,     1},
	/*UPDT*/	{  0,    0,    0,    0,    0,    0,    0,    0,   0,    0,    0,    0,     1}
	};

	assert((unsigned int)priv_data->state < (unsigned int)POP3_STATE_MAX);

	pop3_request_t *req = STAILQ_FIRST(&priv_data->request_list);
	if(req)
	{
		assert((unsigned int)req->req_code < (unsigned int)POP3_COMMAND_MAX);
		return check_table[priv_data->state][req->req_code];
	}
	else if(priv_data->state == POP3_STATE_INIT)
	{
		return 1;
	}
	return 1;
}

void pop3_state_transfer(pop3_data_t *priv_data)
{
	static int transfer_table[POP3_COMMAND_MAX][POP3_STATE_MAX] = 
	{
				/*POP3_STATE_INIT		POP3_STATE_AUTHORIZATION		POP3_STATE_TRANSACTION		POP3_STATE_UPDATE*/
	/*USER*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*PASS*/	{POP3_STATE_INIT,		POP3_STATE_TRANSACTION,			POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*APOP*/	{POP3_STATE_INIT,		POP3_STATE_TRANSACTION,			POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*LIST*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*RETR*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*DELE*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},	
	/*UIDL*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},	
	/*TOP*/		{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},	
	/*STAT*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*QUIT*/	{POP3_STATE_INIT,		POP3_STATE_INIT,				POP3_STATE_UPDATE,			POP3_STATE_UPDATE},
	/*NOOP*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*RSET*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE},
	/*UNKW*/	{POP3_STATE_INIT,		POP3_STATE_AUTHORIZATION,		POP3_STATE_TRANSACTION,		POP3_STATE_UPDATE}
	};
	pop3_response_t *res = STAILQ_FIRST(&priv_data->response_list);
	assert((unsigned int)priv_data->state < (unsigned int)POP3_STATE_MAX);
	assert(res != NULL && res->res_code==POP3_RESPONSE_OK);
	
	pop3_request_t *req = STAILQ_FIRST(&priv_data->request_list);
	if(req)
	{
		assert((unsigned int)req->req_code < (unsigned int)POP3_COMMAND_MAX);
		int new_state = transfer_table[req->req_code][priv_data->state];
		pop3_debug(debug_pop3_server, "transfer state from %d to %d\n", priv_data->state, new_state);
		priv_data->state = new_state;
	}
	else if(priv_data->state == POP3_STATE_INIT)
	{
		pop3_debug(debug_pop3_server, "transfer state from %d to %d\n", POP3_STATE_INIT, POP3_STATE_AUTHORIZATION);
		priv_data->state = POP3_STATE_AUTHORIZATION;
	}
}
