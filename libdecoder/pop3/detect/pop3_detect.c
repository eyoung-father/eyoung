#include <stdlib.h>
#include <assert.h>

#include "pop3_type.h"
#include "pop3_detect.h"
#include "pop3_util.h"
#include "pop3_mem.h"

#define ENTER_FUNC	pop3_debug(debug_pop3_detect, "\n=============ENTER %s=============\n", __FUNCTION__)
#define EXIT_FUNC	pop3_debug(debug_pop3_detect, "=============EXIT %s=============\n\n", __FUNCTION__)
int pop3_state_check(engine_work_t *link, engine_work_event_t *event)
{
	ENTER_FUNC;
	pop3_data_t *predefined = (pop3_data_t*)(link->predefined);
	pop3_userdefined_t *userdefined = (pop3_userdefined_t*)(link->user_defined);
	pop3_request_t *request = *(pop3_request_t**)(event->predefined);

	if(!predefined || !userdefined || !request)
	{
		pop3_debug(debug_pop3_detect, "null pop3_data(%p) or null pop3_userdefined(%p) or null request(%p)\n", predefined, userdefined, request);
		EXIT_FUNC;
		return 0;
	}

	static int check_table[POP3_STATE_MAX][POP3_COMMAND_MAX] = 
	{
				/*USER, PASS, APOP, LIST, RETR, DELE, UIDL, TOP, STAT, QUIT, NOOP, RSET, UNKNOWN*/
	/*INIT*/	{  0,    0,    0,    0,    0,    0,    0,    0,   0,    0,    0,    0,     1},
	/*AUTH*/	{  1,    1,    1,    0,    0,    0,    0,    0,   0,    1,    0,    0,     1},
	/*TRAN*/	{  0,    0,    0,    1,    1,    1,    1,    1,   1,    1,    1,    1,     1},
	/*UPDT*/	{  0,    0,    0,    0,    0,    0,    0,    0,   0,    0,    0,    0,     1}
	};
	
	assert(userdefined->state < POP3_STATE_MAX);

	if(request)
	{
		assert(request->req_code < POP3_COMMAND_MAX);
		pop3_debug(debug_pop3_detect, "current state: %s, current req: %s, check result: %s\n",
			pop3_state_name(userdefined->state),
			pop3_req_type_name(request->req_code),
			check_table[userdefined->state][request->req_code]?"TRUE":"FALSE");
		EXIT_FUNC;
		return !check_table[userdefined->state][request->req_code];
	}
	else if(userdefined->state == POP3_STATE_INIT)
	{
		EXIT_FUNC;
		return 0;
	}
	EXIT_FUNC;
	return 1;
}

void pop3_state_transfer(engine_work_t *link, engine_work_event_t *event)
{
	ENTER_FUNC;
	pop3_data_t *predefined = (pop3_data_t*)(link->predefined);
	pop3_userdefined_t *userdefined = (pop3_userdefined_t*)(link->user_defined);
	pop3_cmd_t *cmd = *(pop3_cmd_t**)(event->predefined);

	if(!predefined || !userdefined || !cmd)
	{
		pop3_debug(debug_pop3_detect, "null pop3_data(%p) or null pop3_userdefined(%p) or null cmd(%p)\n", predefined, userdefined, cmd);
		EXIT_FUNC;
		return;
	}
	
	pop3_request_t *request = cmd->req;
	pop3_response_t *response = cmd->res;
	if(!response)
	{
		pop3_debug(debug_pop3_detect, "null response\n");
		EXIT_FUNC;
		return;
	}
	
	if(response->res_code != POP3_RESPONSE_OK)
	{
		pop3_debug(debug_pop3_detect, "no need do state tranfer for negative response\n");
		EXIT_FUNC;
		return;
	}

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
	assert(userdefined->state < POP3_STATE_MAX);
	
	if(request)
	{
		assert(request->req_code < POP3_COMMAND_MAX);
		int new_state = transfer_table[request->req_code][userdefined->state];
		pop3_debug(debug_pop3_detect, "transfer state from %s to %s\n", pop3_state_name(userdefined->state), pop3_state_name(new_state));
		userdefined->state = new_state;
	}
	else if(userdefined->state == POP3_STATE_INIT)
	{
		pop3_debug(debug_pop3_detect, "transfer state from %s to %s\n", pop3_state_name(POP3_STATE_INIT), pop3_state_name(POP3_STATE_AUTHORIZATION));
		userdefined->state = POP3_STATE_AUTHORIZATION;
	}
	else
	{
		pop3_debug(debug_pop3_detect, "current state is %s, but get null request\n", pop3_state_name(userdefined->state));
	}
	EXIT_FUNC;
}
