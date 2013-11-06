#ifndef POP3_TYPE_H
#define POP3_TYPE_H 1

#include "ey_queue.h"
#include "pop3_decode.h"
#include "libengine.h"

typedef struct pop3_string
{
	char *str;
	int str_len;
}pop3_string_t;

/*
 * Client Message Type
 */
typedef enum pop3_req_type
{
	POP3_COMMAND_USER=0,
	POP3_COMMAND_PASS,
	POP3_COMMAND_APOP,
	POP3_COMMAND_LIST,
	POP3_COMMAND_RETR,
	POP3_COMMAND_DELE,
	POP3_COMMAND_UIDL,
	POP3_COMMAND_TOP,
	POP3_COMMAND_STAT,
	POP3_COMMAND_QUIT,
	POP3_COMMAND_NOOP,
	POP3_COMMAND_RSET,
	POP3_COMMAND_UNKNOWN,

	POP3_COMMAND_MAX
}pop3_req_type_t;

static inline const char* pop3_req_type_name(pop3_req_type_t type)
{
	switch(type)
	{
		case POP3_COMMAND_USER:
			return "USER";
		case POP3_COMMAND_PASS:
			return "PASS";
		case POP3_COMMAND_APOP:
			return "APOP";
		case POP3_COMMAND_LIST:
			return "LIST";
		case POP3_COMMAND_RETR:
			return "RETR";
		case POP3_COMMAND_DELE:
			return "DELE";
		case POP3_COMMAND_UIDL:
			return "UIDL";
		case POP3_COMMAND_TOP:
			return "TOP";
		case POP3_COMMAND_STAT:
			return "STAT";
		case POP3_COMMAND_QUIT:
			return "QUIT";
		case POP3_COMMAND_NOOP:
			return "NOOP";
		case POP3_COMMAND_RSET:
			return "RSET";
		case POP3_COMMAND_UNKNOWN:
		case POP3_COMMAND_MAX:
		default:
			return "Unknown";
	}
}

typedef struct pop3_req_user
{
	int len;
	char *user;
}pop3_req_user_t;

typedef struct pop3_req_pass
{
	int len;
	char *pass;
}pop3_req_pass_t;

typedef struct pop3_req_apop
{
	int len;
	char *pass;
}pop3_req_apop_t;

typedef struct pop3_req_list
{
	int mail;
}pop3_req_list_t;

typedef struct pop3_req_retr
{
	int mail;
}pop3_req_retr_t;

typedef struct pop3_req_dele
{
	int mail;
}pop3_req_dele_t;

typedef struct pop3_req_uidl
{
	int mail;
}pop3_req_uidl_t;

typedef struct pop3_req_top
{
	int mail;
	int lines;
}pop3_req_top_t;

typedef struct pop3_req_unknown
{
	int len;
	char *name;
}pop3_req_unknown_t;

typedef struct pop3_req_arg
{
	char *arg;
	int len;
	STAILQ_ENTRY(pop3_req_arg) next;
}pop3_req_arg_t;

typedef STAILQ_HEAD(pop3_req_arg_list, pop3_req_arg) pop3_req_arg_list_t;

typedef int pop3_req_stat_t;
typedef int pop3_req_quit_t;
typedef int pop3_req_noop_t;
typedef int pop3_req_rset_t;

typedef struct pop3_request
{
	pop3_req_type_t req_code;
	STAILQ_ENTRY(pop3_request) next;
	union
	{
		pop3_req_user_t user;
		pop3_req_pass_t pass;
		pop3_req_apop_t apop;
		pop3_req_list_t list;
		pop3_req_retr_t retr;
		pop3_req_dele_t dele;
		pop3_req_uidl_t uidl;
		pop3_req_top_t  top ;
		pop3_req_stat_t stat;
		pop3_req_quit_t quit;
		pop3_req_noop_t noop;
		pop3_req_rset_t rset;
		pop3_req_unknown_t unknown;
	};
}pop3_request_t;

typedef STAILQ_HEAD(pop3_requests, pop3_request) pop3_requests_t;

/*
 * Server Message Type
 */
typedef enum pop3_res_type
{
	POP3_RESPONSE_OK=0,
	POP3_RESPONSE_ERROR,

	POP3_RESPONSE_MAX
}pop3_res_type_t;

static inline const char *pop3_res_type_name(pop3_res_type_t type)
{
	switch(type)
	{
		case POP3_RESPONSE_OK:
			return "OK";
		case POP3_RESPONSE_ERROR:
			return "ERROR";
		case POP3_RESPONSE_MAX:
		default:
			return "Unknown";
	}
}

typedef struct pop3_line
{
	int line_len;
	char *line;
	STAILQ_ENTRY(pop3_line) next;
}pop3_line_t;

typedef STAILQ_HEAD(pop3_res_content, pop3_line) pop3_res_content_t;

typedef struct pop3_response
{
	pop3_res_type_t res_code;
	int len;
	char *msg;
	pop3_res_content_t content;
	STAILQ_ENTRY(pop3_response) next;
}pop3_response_t;

typedef STAILQ_HEAD(pop3_responses, pop3_response) pop3_responses_t;

/*
 * pop3 session command
 */
typedef struct pop3_cmd
{
	pop3_request_t *req;
	pop3_response_t *res;
	STAILQ_ENTRY(pop3_cmd) next;
}pop3_cmd_t;

typedef STAILQ_HEAD(pop3_cmd_list, pop3_cmd) pop3_cmd_list_t;

/*
 * pop3 parser
 */
typedef struct pop3_parser
{
	void *parser;
	void *lexier;

	char *saved;
	int saved_len;
	char last_frag;
	char greedy;
}pop3_parser_t;

/*
 * pop3 private data
 */
typedef struct pop3_data
{
	pop3_handler_t decoder;
	engine_work_t *engine_work;

	/*client ==> server*/
	pop3_parser_t request_parser;
	pop3_requests_t request_list;

	/*server ==> client*/
	pop3_parser_t response_parser;
	pop3_responses_t response_list;

	/*pop3 session*/
	pop3_cmd_list_t cmd_list;
}pop3_data_t;
#endif
