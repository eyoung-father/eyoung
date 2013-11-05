#include <stdlib.h>
#include <assert.h>

#include "pop3_type.h"
#include "pop3_detect.h"
#include "pop3_util.h"
#include "pop3_mem.h"

int pop3_do_weak_password_check(void *priv_data)
{
	pop3_debug(debug_pop3_detect, "do weak password check\n");
	return 0;
}

int pop3_do_brute_force_check(void *priv_data)
{
	pop3_debug(debug_pop3_detect, "do auth brute force check\n");
	return 0;
}

int pop3_state_check(void *link)
{
	pop3_data_t *priv_data = (pop3_data_t*)link;
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
		pop3_debug(debug_pop3_detect, "current state: %s, current req: %s, check result: %s\n",
			pop3_state_name(priv_data->state),
			pop3_req_type_name(req->req_code),
			check_table[priv_data->state][req->req_code]?"TRUE":"FALSE");
		return check_table[priv_data->state][req->req_code];
	}
	else if(priv_data->state == POP3_STATE_INIT)
	{
		return 1;
	}
	return 1;
}

void pop3_state_transfer(void *link)
{
	pop3_data_t *priv_data = (pop3_data_t*)link;
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
		pop3_debug(debug_pop3_detect, "transfer state from %s to %s\n", pop3_state_name(priv_data->state), pop3_state_name(new_state));
		priv_data->state = new_state;
	}
	else if(priv_data->state == POP3_STATE_INIT)
	{
		pop3_debug(debug_pop3_detect, "transfer state from %s to %s\n", pop3_state_name(POP3_STATE_INIT), pop3_state_name(POP3_STATE_AUTHORIZATION));
		priv_data->state = POP3_STATE_AUTHORIZATION;
	}
}
