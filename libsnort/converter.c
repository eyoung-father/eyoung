#ifdef SNORT_DEBUG

#include <assert.h>
#include "snort_info.h"
#include "snort_signature.h"
#include "snort_parser.h"
#include "snort_lexer.h"

static FILE *idmap_fp;

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
	int start_id;
	char *protocol_name;
	char *file_name;
	FILE *fp;
}snort_ai_info_t;

static snort_ai_info_t ai_info[SNORT_AI_MAX] = 
{
	{1,		"ftp",		"bw_rules/ftp.rule",		NULL},
	{1,		"telnet",	"bw_rules/telnet.rule",		NULL},
	{1,		"smtp",		"bw_rules/smtp.rule",		NULL},
	{1,		"dns",		"bw_rules/dns.rule",		NULL},
	{1,		"dhcp",		"bw_rules/dhcp.rule",		NULL},
	{1,		"tftp",		"bw_rules/tftp.rule",		NULL},
	{1,		"finger",	"bw_rules/finger.rule",		NULL},
	{1,		"http",		"bw_rules/http.rule",		NULL},
	{1,		"pop3",		"bw_rules/pop3.rule",		NULL},
	{1,		"sunrpc",	"bw_rules/sunrpc.rule",		NULL},
	{1,		"msrpc",	"bw_rules/msrpc.rule",		NULL},
	{1,		"netbios",	"bw_rules/nbname.rule",		NULL},
	{4000,	"netbios",	"bw_rules/nbds.rule",		NULL},
	{8000,	"netbios",	"bw_rules/smb.rule",		NULL},
	{1,		"imap",		"bw_rules/imap.rule",		NULL},
	{1,		"snmp",		"bw_rules/snmp.rule",		NULL},
	{1,		"ldap",		"bw_rules/ldap.rule",		NULL},
	{1,		"mssql",	"bw_rules/mssql.rule",		NULL},
	{1,		"oracle",	"bw_rules/oracle.rule",		NULL},
	{1,		"mysql",	"bw_rules/mysql.rule",		NULL},
	{1,		"voip",		"bw_rules/voip.rule",		NULL},
	{1,		"tcp",		"bw_rules/tcp.rule",		NULL},
	{1,		"udp",		"bw_rules/udp.rule",		NULL},
	{1,		"icmp",		"bw_rules/icmp.rule",		NULL},
	{1,		"ip",		"bw_rules/ip.rule",			NULL},
};

static void snort_ai_init()
{
	int i;
	for(i=0; i<SNORT_AI_MAX; i++)
	{
		ai_info[i].fp = fopen(ai_info[i].file_name, "w");
		assert(ai_info[i].fp != NULL);
	}

	idmap_fp = fopen("bw_rules/id_map", "a+");
	assert(idmap_fp != NULL);
}

typedef struct snort_app_map
{
	char *prefix;
	int app_id;
}snort_app_map_t;

static const snort_app_map_t app_map[65536] = 
{
	[21]	= {"ftp",		SNORT_AI_FTP},
	[23]	= {"telnet",	SNORT_AI_TELNET},
	[25]	= {"smtp",		SNORT_AI_SMTP},
	[53]	= {"dns",		SNORT_AI_DNS},
	[67]	= {"dhcp",		SNORT_AI_DHCP},
	[69]	= {"tftp",		SNORT_AI_TFTP},
	[79]	= {"finger",	SNORT_AI_FINGER},
	[80]	= {"http",		SNORT_AI_HTTP},
	[110]	= {"pop3",		SNORT_AI_POP3},
	[111]	= {"sunrpc",	SNORT_AI_SUNRPC},
	[135]	= {"msrpc",		SNORT_AI_MSRPC},
	[137]	= {"nbname",	SNORT_AI_NBNAME},
	[138]	= {"nbds",		SNORT_AI_NBDS},
	[139]	= {"smb",		SNORT_AI_SMB},
	[143]	= {"imap4",		SNORT_AI_IMAP4},
	[160]	= {"snmp",		SNORT_AI_SNMP},
	[161]	= {"snmp",		SNORT_AI_SNMP},
	[162]	= {"snmp",		SNORT_AI_SNMP},
	[389]	= {"ldap",		SNORT_AI_LDAP},
	[443]	= {"http",		SNORT_AI_HTTP},
	[445]	= {"smb",		SNORT_AI_SMB},
	[1433]	= {"mssql",		SNORT_AI_MSSQL},
	[1434]	= {"mssql",		SNORT_AI_MSSQL},
	[1521]	= {"oracle",	SNORT_AI_ORACLE},
	[3306]	= {"mysql",		SNORT_AI_MYSQL},
	[5060]	= {"sip",		SNORT_AI_SIP},
	[5061]	= {"sip",		SNORT_AI_SIP},
	[8000]	= {"http",		SNORT_AI_HTTP},
	[8008]	= {"http",		SNORT_AI_HTTP},
	[8080]	= {"http",		SNORT_AI_HTTP},
};

int snort_ai(snort_signature_t *signature, 
	char *ret1, int ret1_len, 
	char *ret2, int ret2_len)
{
	assert(signature!=NULL);
	assert(ret1!=NULL && ret1_len>0);
	assert(ret2!=NULL && ret2_len>0);

	snort_port_t *src_port = &signature->src_port;
	snort_port_t *dst_port = &signature->dst_port;
	snort_protocol_t protocol = signature->protocol;
	
	ret1[0] = 0;
	ret2[0] = 0;

	if(src_port->negative || dst_port->negative)
		goto other;
	
	if(app_map[src_port->low_port].prefix)
	{
		snprintf(ret1, ret1_len, "%s_s2c_data", app_map[src_port->low_port].prefix);
		return app_map[src_port->low_port].app_id;
	}

	if(app_map[dst_port->low_port].prefix)
	{
		snprintf(ret1, ret1_len, "%s_c2s_data", app_map[dst_port->low_port].prefix);
		return app_map[dst_port->low_port].app_id;
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
				return SNORT_AI_TCP;
			}
			else if(!src_port->low_port)
			{
				snprintf(ret1, ret1_len, "tcp_any_c2s_data");
				return SNORT_AI_TCP;
			}
			else
			{
				snprintf(ret1, ret1_len, "tcp_any_s2c_data");
				return SNORT_AI_TCP;
			}
			break;
		}
		case SNORT_PROTOCOL_UDP:
		{
			if(!src_port->low_port && !dst_port->low_port)
			{
				snprintf(ret1, ret1_len, "udp_any_c2s_data");
				snprintf(ret2, ret2_len, "udp_any_s2c_data");
				return SNORT_AI_UDP;
			}
			else if(!src_port->low_port)
			{
				snprintf(ret1, ret1_len, "udp_any_c2s_data");
				return SNORT_AI_UDP;
			}
			else
			{
				snprintf(ret1, ret1_len, "udp_any_s2c_data");
				return SNORT_AI_UDP;
			}
			break;
		}
		case SNORT_PROTOCOL_ICMP:
		{
			snprintf(ret1, ret1_len, "icmp_data");
			return SNORT_AI_ICMP;
		}
		case SNORT_PROTOCOL_IP:
		{
			snprintf(ret1, ret1_len, "ip_data");
			return SNORT_AI_IP;
		}
		default:
			assert(0);
	}
	assert(0);
	return 0;
}

static void convert_option(snort_signature_t *signature, int rule_id, FILE *out_fp)
{
	assert(signature!=NULL);
	assert(out_fp!=NULL);
	
	snort_option_t *option = NULL;
	int output_idmap=0;
	int first = 1;
	TAILQ_FOREACH(option, &signature->option_list, link)
	{
		switch(option->type)
		{
			case SNORT_OPTION_TYPE_CONTENT:
			case SNORT_OPTION_TYPE_URICONTENT:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_SID:
			{
				assert(output_idmap==0);
				fprintf(idmap_fp, "%d,%d\n", option->sid.sid, rule_id);
				output_idmap = 1;
				break;
			}
			case SNORT_OPTION_TYPE_DSIZE:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_BYTETEST:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_BYTEJUMP:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_IPPROTO:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_TTL:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_ITYPE:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_ICODE:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_ICMPSEQ:
			{
				if(!first)
					fprintf(out_fp, " and ");
				first = 0;
				break;
			}
			case SNORT_OPTION_TYPE_MSG:
			case SNORT_OPTION_TYPE_NOCASE:
			case SNORT_OPTION_TYPE_OFFSET:
			case SNORT_OPTION_TYPE_DEPTH:
			case SNORT_OPTION_TYPE_WITHIN:
			case SNORT_OPTION_TYPE_DISTANCE:
			case SNORT_OPTION_TYPE_CLASSTYPE:
			case SNORT_OPTION_TYPE_PRIORITY:
			{
				break;
			}
			default:
			{	
				assert(0);
				break;
			}
		}
	}
}

static void convert_signature(snort_signature_t *signature)
{
	assert(signature!=NULL);

	char elm_name1[128] = {0};
	char elm_name2[128] = {0};
	FILE *out_fp = NULL;
	char *protocol = NULL;
	int rule_id = 0;
	int app_id = snort_ai(signature, 
		elm_name1, sizeof(elm_name1),
		elm_name2, sizeof(elm_name2));
	
	out_fp = ai_info[app_id].fp;
	protocol = ai_info[app_id].protocol_name;
	rule_id = ai_info[app_id].start_id++;

	fprintf(out_fp, "idp_signature %d:%s\n", rule_id, protocol);
	fprintf(out_fp, "{\n");
	fprintf(out_fp, "\tsignature: %s(", elm_name1);
	convert_option(signature, rule_id, out_fp);
	fprintf(out_fp, ")\n");
	if(elm_name2[0])
	{
		fprintf(out_fp, "\t| %s(", elm_name2);
		convert_option(signature, rule_id, out_fp);
		fprintf(out_fp, ")\n");
	}
	fprintf(out_fp, "};\n");
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
	
	snort_signature_t *signature = NULL;
	TAILQ_FOREACH(signature, &signature_list, link)
		convert_signature(signature);

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
	
	snort_ai_init();
	
	debug_snort_parser = 1;
	debug_snort_lexer = 1;
	debug_snort_init = 1;
	int i;
	for(i=1; i<argc; i++)
		convert_file(argv[i]);
	return 0;
}
#endif
