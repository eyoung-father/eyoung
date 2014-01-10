%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "pop3.h"
#include "pop3_type.h"
#include "pop3_util.h"
#include "pop3_decode.h"
#include "pop3_mem.h"
#include "pop3_client_parser.h"
#include "pop3_server_parser.h"
#include "pop3_client_lex.h"
#include "pop3_server_lex.h"
#include "pop3_private.h"
#include "pop3_private.h"

#ifdef YY_REDUCTION_CALLBACK
#undef YY_REDUCTION_CALLBACK
#endif
#define YY_REDUCTION_CALLBACK(data,name,id,val)									\
	do																			\
	{																			\
		if(pop3_element_detect(data,name,id,val,								\
			cluster_buffer,cluster_buffer_len)<0)								\
		{																		\
			pop3_debug(debug_pop3_detect, "find attack!\n");					\
			return -1;															\
		}																		\
	}while(0)

#define priv_decoder															\
	((pop3_decoder_t*)(((pop3_data_t*)priv_data)->decoder))

int pop3_cmd_pair_id;
%}
%token TOKEN_CLIENT_USER
%token TOKEN_CLIENT_PASS
%token TOKEN_CLIENT_APOP

%token TOKEN_CLIENT_QUIT
%token TOKEN_CLIENT_NOOP

%token TOKEN_CLIENT_STAT
%token TOKEN_CLIENT_LIST
%token TOKEN_CLIENT_RETR
%token TOKEN_CLIENT_DELE
%token TOKEN_CLIENT_RSET
%token TOKEN_CLIENT_TOP
%token TOKEN_CLIENT_UIDL
%token TOKEN_CLIENT_UNKNOWN
%token TOKEN_CLIENT_STRING

%token TOKEN_CLIENT_NEWLINE
%token TOKEN_CLIENT_CONTINUE

%union
{
	pop3_string_t string;
	pop3_request_t *request;
	pop3_req_arg_list_t arg_list;
}

%type <arg_list>	request_args
%type <request>		noop_command
					quit_command
					uidl_command
					top_command
					rset_command
					dele_command
					retr_command
					list_command
					stat_command
					apop_command
					user_command
					pass_command
					unknown_command
					request

%destructor
{
	pop3_free_req_arg_list(priv_decoder, &$$);
}request_args

%debug
%verbose
%defines "pop3_client_parser.h"
%output "pop3_client_parser.c"
%define api.prefix pop3_client_
%define api.pure full
%define api.push-pull push
%parse-param {void *priv_data}

%start request_list
%%
request_list:
	{
		pop3_data_t *data = (pop3_data_t*)priv_data;
		STAILQ_INIT(&data->request_list);
	}
	| request_list request 
	{
		pop3_data_t *data = (pop3_data_t*)priv_data;
		STAILQ_INSERT_TAIL(&data->request_list, $2, next);
	}
	TOKEN_CLIENT_NEWLINE
	{
		/*do nothing*/
	}
	;

request: user_command
	{
		$$ = $1;
	}
	| pass_command
	{
		$$ = $1;
	}
	| apop_command
	{
		$$ = $1;
	}
	| stat_command
	{
		$$ = $1;
	}
	| list_command
	{
		$$ = $1;
	}
	| retr_command
	{
		$$ = $1;
	}
	| dele_command
	{
		$$ = $1;
	}
	| rset_command
	{
		$$ = $1;
	}
	| top_command
	{
		$$ = $1;
	}
	| uidl_command
	{
		$$ = $1;
	}
	| quit_command
	{
		$$ = $1;
	}
	| noop_command
	{
		$$ = $1;
	}
	| unknown_command
	{
		$$ = $1;
	}
	;

user_command: TOKEN_CLIENT_USER request_args
	{
		if(STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: USER command need follow parameter\n");

		if(STAILQ_FIRST(&$2) && STAILQ_NEXT(STAILQ_FIRST(&$2), next))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: USER command need follow only one parameter\n");

		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		pop3_request_t *req = NULL;
		if(arg)
		{
			req = pop3_alloc_request(priv_decoder, POP3_COMMAND_USER, arg->arg, arg->len);
			if(!req)
			{
				pop3_debug(debug_pop3_client_parser, "failed to alloc USER command\n");
				YYABORT;
			}
			arg->arg = NULL;
			pop3_free_req_arg_list(priv_decoder, &$2);
		}
		else
		{
			req = pop3_alloc_request(priv_decoder, POP3_COMMAND_USER, NULL, 0);
			if(!req)
			{
				pop3_debug(debug_pop3_client_parser, "failed to alloc USER command\n");
				YYABORT;
			}
		}
		$$ = req;
	}
	;

pass_command: TOKEN_CLIENT_PASS request_args
	{
		if(STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: PASS command need follow parameter\n");

		if(STAILQ_FIRST(&$2) && STAILQ_NEXT(STAILQ_FIRST(&$2), next))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: PASS command need follow only one parameter\n");

		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		pop3_request_t *req = NULL;
		if(arg)
		{
			req = pop3_alloc_request(priv_decoder, POP3_COMMAND_PASS, arg->arg, arg->len);
			if(!req)
			{
				pop3_debug(debug_pop3_client_parser, "failed to alloc PASS command\n");
				YYABORT;
			}
			arg->arg = NULL;
			pop3_free_req_arg_list(priv_decoder, &$2);
		}
		else
		{
			req = pop3_alloc_request(priv_decoder, POP3_COMMAND_PASS, NULL, 0);
			if(!req)
			{
				pop3_debug(debug_pop3_client_parser, "failed to alloc PASS command\n");
				YYABORT;
			}
		}
		$$ = req;
	}
	;

apop_command: TOKEN_CLIENT_APOP request_args
	{
		if(STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: APOP command need follow parameter\n");

		if(STAILQ_FIRST(&$2) && STAILQ_NEXT(STAILQ_FIRST(&$2), next))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: APOP command need follow only one parameter\n");

		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		pop3_request_t *req = NULL;
		if(arg)
		{
			req = pop3_alloc_request(priv_decoder, POP3_COMMAND_APOP, arg->arg, arg->len);
			if(!req)
			{
				pop3_debug(debug_pop3_client_parser, "failed to alloc APOP command\n");
				YYABORT;
			}
			arg->arg = NULL;
			pop3_free_req_arg_list(priv_decoder, &$2);
		}
		else
		{
			req = pop3_alloc_request(priv_decoder, POP3_COMMAND_APOP, NULL, 0);
			if(!req)
			{
				pop3_debug(debug_pop3_client_parser, "failed to alloc APOP command\n");
				YYABORT;
			}
		}
		$$ = req;
	}
	;

stat_command: TOKEN_CLIENT_STAT request_args
	{
		if(!STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: STAT command do not follow any parameter\n");
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_STAT);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc STAT command\n");
			YYABORT;
		}
		pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

list_command: TOKEN_CLIENT_LIST request_args
	{
		int mail = 0;
		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		if(arg)
		{
			int error = 0;
			mail = pop3_parse_integer(arg->arg, &error);
			if(mail<0 || error)
			{
				mail = 0;
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: bad parameter %s, LIST command need positive mail index\n", arg->arg);
			}

			if(STAILQ_NEXT(arg, next))
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: LIST command need ONE or ZERO mail index as its parameter\n");
		}
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_LIST, mail);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc LIST command\n");
			YYABORT;
		}

		if(arg)
			pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

retr_command: TOKEN_CLIENT_RETR request_args
	{
		int mail = 0;
		if(STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: RETR command need ONE mail index as parameter\n");

		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		if(arg)
		{
			int error = 0;
			mail = pop3_parse_integer(arg->arg, &error);
			if(mail<0 || error)
			{
				mail = 0;
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: bad parameter %s, RETR command need positive mail index\n", arg->arg);
			}

			if(STAILQ_NEXT(arg, next))
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: RETR command need ONE or ZERO mail index as its parameter\n");
		}
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_RETR, mail);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc RETR command\n");
			YYABORT;
		}

		if(arg)
			pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

dele_command: TOKEN_CLIENT_DELE request_args
	{
		int mail = 0;
		if(STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: DELE command need ONE mail index as parameter\n");

		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		if(arg)
		{
			int error = 0;
			mail = pop3_parse_integer(arg->arg, &error);
			if(mail<0 || error)
			{
				mail = 0;
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: bad parameter %s, DELE command need positive mail index\n", arg->arg);
			}

			if(STAILQ_NEXT(arg, next))
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: DELE command need ONE or ZERO mail index as its parameter\n");
		}
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_DELE, mail);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc DELE command\n");
			YYABORT;
		}

		if(arg)
			pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

rset_command: TOKEN_CLIENT_RSET request_args
	{
		if(!STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: RSET command do not follow any parameter\n");
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_RSET);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc RSET command\n");
			YYABORT;
		}
		pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

top_command: TOKEN_CLIENT_TOP request_args
	{
		int mail = 0, lines = 0;
		if(STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: TOP command need TWO parameters\n");

		pop3_req_arg_t *arg1 = STAILQ_FIRST(&$2);
		if(arg1)
		{
			int error = 0;
			mail = pop3_parse_integer(arg1->arg, &error);
			if(mail<0 || error)
			{
				mail = 0;
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: bad parameter %s, TOP command need positive mail index\n", arg1->arg);
			}

			pop3_req_arg_t *arg2 = STAILQ_NEXT(arg1, next);
			if(!arg2)
			{
				lines = 0;
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: TOP command need second parameter as lines\n");
			}
			else
			{
				error = 0;
				lines = pop3_parse_integer(arg2->arg, &error);
				if(lines<0 || error)
				{
					lines = 0;
					pop3_abnormal(debug_pop3_client_parser, "Abnormal: bad parameter %s, TOP command need positive mail index\n", arg2->arg);
				}

				if(STAILQ_NEXT(arg2, next))
					pop3_abnormal(debug_pop3_client_parser, "Abnormal: too many parameters, TOP command need TWO parameters\n");
			}
		}
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_TOP, mail, lines);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc TOP command\n");
			YYABORT;
		}

		if(arg1)
			pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

uidl_command: TOKEN_CLIENT_UIDL request_args
	{
		int mail = 0;
		pop3_req_arg_t *arg = STAILQ_FIRST(&$2);
		if(arg)
		{
			int error = 0;
			mail = pop3_parse_integer(arg->arg, &error);
			if(mail<0 || error)
			{
				mail = 0;
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: bad parameter %s, UIDL command need positive mail index\n", arg->arg);
			}

			if(STAILQ_NEXT(arg, next))
				pop3_abnormal(debug_pop3_client_parser, "Abnormal: UIDL command need ONE or ZERO mail index as its parameter\n");
		}
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_UIDL, mail);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc UIDL command\n");
			YYABORT;
		}

		if(arg)
			pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

quit_command: TOKEN_CLIENT_QUIT request_args
	{
		if(!STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: QUIT command do not follow any parameter\n");
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_QUIT);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc QUIT command\n");
			YYABORT;
		}
		pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

noop_command: TOKEN_CLIENT_NOOP request_args
	{
		if(!STAILQ_EMPTY(&$2))
			pop3_abnormal(debug_pop3_client_parser, "Abnormal: NOOP command do not follow any parameter\n");
		
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_NOOP);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc NOOP command\n");
			YYABORT;
		}
		pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

unknown_command: TOKEN_CLIENT_UNKNOWN request_args
	{
		char *name = (char*)pop3_malloc(yylval.string.str_len+1);
		if(!name)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc unknown command name\n");
			YYABORT;
		}
		memcpy(name, yylval.string.str, yylval.string.str_len);
		name[yylval.string.str_len] = '\0';

		pop3_abnormal(debug_pop3_client_parser, "Abnormal: UNKNOWN command %s\n", name);
		pop3_request_t *req = pop3_alloc_request(priv_decoder, POP3_COMMAND_UNKNOWN, name);
		if(!req)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc UNKNOWN command\n");
			pop3_free(name);
			YYABORT;
		}
		pop3_free_req_arg_list(priv_decoder, &$2);
		$$ = req;
	}
	;

request_args: 
	{
		STAILQ_INIT(&$$);
	}
	| request_args TOKEN_CLIENT_STRING
	{
		pop3_req_arg_t *arg = pop3_alloc_req_arg(priv_decoder, yylval.string.str, yylval.string.str_len);
		if(!arg)
		{
			pop3_debug(debug_pop3_client_parser, "failed to alloc req arg\n");
			YYABORT;
		}
		STAILQ_INSERT_TAIL(&$1, arg, next);
		STAILQ_CONCAT(&$$, &$1);
	}
	;
%%
int parse_pop3_client_stream(pop3_data_t *priv, const char *buf, size_t buf_len, int last_frag)
{
	pop3_client_pstate *parser = (pop3_client_pstate*)priv->request_parser.parser;
	yyscan_t lexier = (yyscan_t)priv->request_parser.lexier;
	YY_BUFFER_STATE input = NULL;
	int token = 0, parser_ret = 0;
	POP3_CLIENT_STYPE value;

	yydebug = debug_pop3_client_parser;
	priv->request_parser.last_frag = last_frag;
	input = pop3_client_scan_stream(buf, buf_len, priv);
	if(!input)
	{
		pop3_debug(debug_pop3_client_parser, "create pop3 client stream buffer failed\n");
		return 1;
	}

	while(1)
	{
		memset(&value, 0, sizeof(value));
		token = pop3_client_lex(&value, lexier);
		if(token == TOKEN_CLIENT_CONTINUE)
			break;
		parser_ret = pop3_client_push_parse(parser, token, &value, (void*)priv);
		if(parser_ret != YYPUSH_MORE)
			break;
	}
	pop3_client_delete_buffer(input, lexier);

	if(parser_ret != YYPUSH_MORE && parser_ret != 0)
	{
		pop3_debug(debug_pop3_client_parser, "find error while parsing pop3 client stream\n");
		return 2;
	}
	return 0;
}

void pop3_client_register(pop3_decoder_t *decoder)
{
	assert(decoder!=NULL);
	assert(decoder->engine!=NULL);

	engine_t engine = decoder->engine;
	int index = 0;
	for(index=0; index<YYNNTS; index++)
	{
		const char *name = yytname[YYNTOKENS + index];
		if(!name || name[0]=='$' || name[0]=='@')
			continue;
		yytid[YYNTOKENS + index] = ey_engine_find_event(engine, name);
		if(yytid[YYNTOKENS + index] >= 0)
			pop3_debug(debug_pop3_server_parser, "client event id of %s is %d\n", name, yytid[YYNTOKENS + index]);
		else
			pop3_debug(debug_pop3_server_parser, "failed to register client event %s\n", name);
	}

	pop3_cmd_pair_id = ey_engine_find_event(engine, "cmd_pair");
}
#undef priv_decoder
