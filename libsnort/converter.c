#ifdef SNORT_DEBUG

#include <assert.h>
#include "snort_info.h"
#include "snort_signature.h"
#include "snort_parser.h"
#include "snort_lexer.h"

typedef enum snort_ai
{
	SNORT_AI_FTP = 0,
	SNORT_AI_TELNET,
	SNORT_AI_SMTP,
	SNORT_AI_DNS,
	SNORT_AI_DHCP,
	SNORT_AI_TFTP,
	SNORT_AI_FINGER,
	SNORT_AI_HTTP,
	SNORT_AI_POP3,
	SNORT_AI_SUNRPC,
	SNORT_AI_MSRPC,
	SNORT_AI_NBNAME,
	SNORT_AI_NBDS,
	SNORT_AI_SMB,
	SNORT_AI_IMAP4,
	SNORT_AI_SNMP,
	SNORT_AI_LDAP,
	SNORT_AI_MSSQL,
	SNORT_AI_ORACLE,
	SNORT_AI_MYSQL,
	SNORT_AI_SIP,
	SNORT_AI_TCP,
	SNORT_AI_UDP,
	SNORT_AI_ICMP,
	SNORT_AI_IP,
	SNORT_AI_MAX
}snort_ai_t;

typedef struct snort_ai_info
{
	int id;
	char *name;
	FILE *fp;
}snort_ai_info_t;

static int snort_ai_init()
{
	static snort_ai_info_t ai_info[SNORT_AI_MAX] = 
	{
		{1,	"ftp",		NULL},
		{1,	"telnet",	NULL},
		{1,	"smtp",		NULL},
		{1,	"dns",		NULL},
		{1,	"dhcp",		NULL},
		{1,	"tftp",		NULL},
		{1,	"finger",	NULL},
		{1,	"http",		NULL},
		{1,	"pop3",		NULL},
		{1,	"sunrpc",	NULL},
		{1,	"msrpc",	NULL},
		{1,	"netbios",	NULL},
		{1,	"netbios",	NULL},
	};
}

int convert_element_name(snort_signature_t *signature, 
	char *ret1, int ret1_len, 
	char *ret2, int ret2_len)
{
	static char *app_map[65536] = 
	{
		[21]	= "ftp",
		[23]	= "telnet",
		[25]	= "smtp",
		[53]	= "dns",
		[67]	= "dhcp",
		[69]	= "tftp",
		[79]	= "finger",
		[80]	= "http",
		[110]	= "pop3",
		[111]	= "sunrpc",
		[135]	= "msrpc",
		[137]	= "nbname",
		[138]	= "nbds",
		[139]	= "smb",
		[143]	= "imap4",
		[160]	= "snmp",
		[161]	= "snmp",
		[162]	= "snmp",
		[389]	= "ldap",
		[443]	= "http",
		[1433]	= "mssql",
		[1434]	= "mssql",
		[1521]	= "oracle",
		[3306]	= "mysql",
		[5060]	= "sip",
		[5061]	= "sip",
		[8000]	= "http",
		[8008]	= "http",
		[8080]	= "http",
	};

	assert(signature!=NULL);
	assert(ret1!=NULL && ret1_len>0);
	assert(ret2!=NULL && ret2_len>0);

	snort_port_t *src_port = &signature->src_port;
	snort_port_t *dst_port = &signature->dst_port;
	snort_protocol_t protocol = signature->protocol;
	
	if(src_port->negative || dst_port->negative)
		goto other;
	
	if(app_map[src_port->low_port])
	{
		snprintf(ret1, ret1_len, "%s_s2c_data", app_map[src_port->low_port]);
		return 1;
	}

	if(app_map[dst_port->low_port])
	{
		snprintf(ret1, ret1_len, "%s_c2s_data", app_map[dst_port->low_port]);
		return 1;
	}

other:
	switch(protocol)
	{
		case SNORT_PROTOCOL_TCP:
		{
			if(!src_port->low_port && !dst_port->low_port)
			{
				snprintf(ret1, ret1_len, "tcp_any_c2s_data");
				snprintf(ret2, ret2_len, "tcp_any_s2c_data");
				return 2;
			}
			else if(!src_port->low_port)
			{
				snprintf(ret1, ret1_len, "tcp_any_c2s_data");
				return 1;
			}
			else
			{
				snprintf(ret1, ret1_len, "tcp_any_s2c_data");
				return 1;
			}
			break;
		}
		case SNORT_PROTOCOL_UDP:
		{
			if(!src_port->low_port && !dst_port->low_port)
			{
				snprintf(ret1, ret1_len, "udp_any_c2s_data");
				snprintf(ret2, ret2_len, "udp_any_s2c_data");
				return 2;
			}
			else if(!src_port->low_port)
			{
				snprintf(ret1, ret1_len, "udp_any_c2s_data");
				return 1;
			}
			else
			{
				snprintf(ret1, ret1_len, "udp_any_s2c_data");
				return 1;
			}
			break;
		}
		case SNORT_PROTOCOL_ICMP:
		{
			snprintf(ret1, ret1_len, "icmp_data");
			return 1;
		}
		case SNORT_PROTOCOL_IP:
		{
			snprintf(ret1, ret1_len, "ip_data");
			return 1;
		}
		default:
			assert(0);
	}
	assert(0);
	return 0;
}

static int convert_file(const char *filename)
{
	FILE *fp = NULL;
	yyscan_t lexer = NULL;
	snort_pstate *pstate = NULL;
	SNORT_STYPE sval;
	SNORT_LTYPE lval = {1,1,1,1};
	int token = 0, pstate_ret = 0;

	if(!filename)
	{
		snort_init_error("null filename\n");
		return -1;
	}
	
	if((fp=fopen(filename, "r")) == NULL)
	{
		snort_init_error("open file %s failed\n", filename);
		goto failed;
	}

	if(snort_lex_init(&lexer))
	{
		snort_init_error("alloc lexer failed\n");
		goto failed;
	}
	snort_set_in(fp, lexer);
	
	pstate = snort_pstate_new();
	if(!pstate)
	{
		snort_init_error("alloc pstate failed\n");
		goto failed;
	}
	
	while(1)
	{
		token = snort_lex(&sval, &lval, lexer);
		pstate_ret = snort_push_parse(pstate, token, &sval, &lval);
		if(pstate_ret != YYPUSH_MORE)
			break;
	}

	if(pstate_ret != YYPUSH_MORE && pstate_ret != 0)
		snort_init_error("find error while parsing %s\n", filename);
	else
		snort_init_debug("parse %s successfully\n", filename);
	
failed:
	if(pstate)
		snort_pstate_delete(pstate);
	if(lexer)
		snort_lex_destroy(lexer);
	if(fp)
		fclose(fp);
	return 0;
}

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		snort_init_error("need snort filename list as the parameter\n");
		return -1;
	}
	
	debug_snort_parser = 1;
	debug_snort_lexer = 1;
	debug_snort_init = 1;
	int i;
	for(i=1; i<argc; i++)
		convert_file(argv[i]);
	return 0;
}
#endif
