%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "snort_mem.h"
#include "snort_signature.h"
#include "snort_parser.h"
#include "snort_info.h"
int snort_error(SNORT_LTYPE *loc, const char *format, ...);

#undef yydebug
#define yydebug debug_snort_parser
%}

%token TOKEN_LPAREN			"("
%token TOKEN_RPAREN			")"
%token TOKEN_MSG			"msg"
%token TOKEN_COLON			":"
%token TOKEN_STRING			"[string]"
%token TOKEN_BYTE_STRING	"string"
%token TOKEN_SEMI			";"
%token TOKEN_COMMA			","
%token TOKEN_CONTENT		"content"
%token TOKEN_NEG			"!"
%token TOKEN_NOCASE			"nocase"
%token TOKEN_OFFSET			"offset"
%token TOKEN_NUMBER			"[number]"
%token TOKEN_DEPTH			"depth"
%token TOKEN_WITHIN			"within"
%token TOKEN_DISTANCE		"distance"
%token TOKEN_CLASSTYPE		"classtype"
%token TOKEN_SID			"sid"
%token TOKEN_PRIORITY		"priority"
%token TOKEN_URICONTENT		"uricontent"
%token TOKEN_DSIZE			"dsize"
%token TOKEN_LT				"<"
%token TOKEN_GT				">"
%token TOKEN_IPPROTO		"ip_proto"
%token TOKEN_IGMP			"igmp"
%token TOKEN_TTL			"ttl"
%token TOKEN_LTEQ			"<="
%token TOKEN_GTEQ			">="
%token TOKEN_EQ				"="
%token TOKEN_ITYPE			"itype"
%token TOKEN_ICODE			"icode"
%token TOKEN_ICMPSEQ		"icmp_seq"
%token TOKEN_BYTETEST		"byte_test"
%token TOKEN_BITAND			"&"
%token TOKEN_BITOR			"^"
%token TOKEN_RELATIVE		"relative"
%token TOKEN_BIG			"big"
%token TOKEN_LITTLE			"little"
%token TOKEN_DEC			"dec"
%token TOKEN_HEX			"hex"
%token TOKEN_OCT			"oct"
%token TOKEN_BYTEJUMP		"byte_jump"
%token TOKEN_MULTIPLIER		"multiplier"
%token TOKEN_ALIGN			"align"
%token TOKEN_FROMBEGIN		"from_beginning"
%token TOKEN_POSTOFFSET		"post_offset"
%token TOKEN_ALERT			"alert"
%token TOKEN_PASS			"pass"
%token TOKEN_DROP			"drop"
%token TOKEN_REJECT			"reject"
%token TOKEN_LOG			"log"
%token TOKEN_DYNAMIC		"dynamic"
%token TOKEN_ACTIVATE		"activate"
%token TOKEN_SDROP			"sdrop"
%token TOKEN_TCP			"tcp"
%token TOKEN_UDP			"udp"
%token TOKEN_ICMP			"icmp"
%token TOKEN_IP				"ip"
%token TOKEN_ANY			"any"
%token TOKEN_SARROW			"->"
%token TOKEN_DARROW			"<>"
%token TOKEN_ACCESS			"Access Control"
%token TOKEN_MISC			"Misc"
%token TOKEN_WEB			"Web Attack"
%token TOKEN_DDOS			"DoS/DDoS"
%token TOKEN_BUFFER			"Buffer Overflow"
%token TOKEN_MAIL			"Mail"
%token TOKEN_SCAN			"Scan"
%token TOKEN_BACKDOOR		"Backdoor/Trojan"
%token TOKEN_VIRUS			"Virus/Worm"

%union
{
	int number;
	char *string;
	snort_action_t action;
	snort_protocol_t protocol;
	snort_ip_t ip;
	snort_port_t port;
	unsigned short us;
	snort_direction_t direction;
	snort_option_t option;
	snort_option_list_t option_list;
	snort_signature_t signature;
	snort_signature_list_t signature_list;
	snort_classtype_t classtype;
	snort_operator_t operator;
	snort_option_bytetest_t bytetest;
	snort_option_bytejump_t bytejump;
}

%type	<string>			TOKEN_STRING
%type	<number>			TOKEN_NUMBER
%type	<action>			action
%type	<protocol>			protocol
%type	<ip>				src_ip
%type	<ip>				dst_ip
%type	<port>				dst_port
%type	<port>				src_port
%type	<number>			negative_opt
%type	<us>				max_port_opt
%type	<direction>			direction
%type	<option>			item
%type	<option>			msg_item
%type	<option>			nocase_item
%type	<option>			content_item
%type	<option>			uricontent_item
%type	<option>			offset_item
%type	<option>			depth_item
%type	<option>			distance_item
%type	<option>			within_item
%type	<option>			classtype_item
%type	<option>			sid_item
%type	<option>			priority_item
%type	<option>			dsize_item
%type	<option>			bytetest_item
%type	<option>			bytejump_item
%type	<option>			ipproto_item
%type	<option>			ttl_item
%type	<option>			itype_item
%type	<option>			icode_item
%type	<option>			icmpseq_item
%type	<option_list>		body
%type	<signature_list>	signature_list
%type	<signature>			signature
%type	<classtype>			classtype
%type	<operator>			dsize_operator_opt
%type	<operator>			ttl_operator_opt
%type	<operator>			itype_operator_opt
%type	<operator>			icode_operator_opt
%type	<operator>			bytetest_operator
%type	<bytetest>			bytetest_option_list
%type	<bytetest>			bytetest_option
%type	<bytetest>			bytetest_option_relative
%type	<bytetest>			bytetest_option_endian
%type	<bytetest>			bytetest_option_string
%type	<bytejump>			bytejump_option_list
%type	<bytejump>			bytejump_option
%type	<bytejump>			bytejump_option_relative
%type	<bytejump>			bytejump_option_endian
%type	<bytejump>			bytejump_option_multiplier
%type	<bytejump>			bytejump_option_align
%type	<bytejump>			bytejump_option_frombegin
%type	<bytejump>			bytejump_option_postoffset

%debug
%verbose
%locations
%defines "snort_parser.h"
%output "snort_parser.c"
%define api.prefix snort_
%define api.pure full
%define api.push-pull push

%start signature_list
%%
empty:
	;

signature_list:
	empty
	{
		TAILQ_INIT(&$$);
	}
	| signature_list signature
	{
		snort_signature_t *signature = (snort_signature_t*)snort_malloc(sizeof(snort_signature_t));
		if(!signature)
		{
			snort_parser_error("alloc signature failed\n");
			assert(0);
		}
		*signature = $2;
		TAILQ_INSERT_TAIL(&$$, signature, link);
	}
	;

signature:
	action protocol src_ip src_port direction dst_ip dst_port TOKEN_LPAREN body TOKEN_RPAREN
	{
		$$.action = $1;
		$$.protocol = $2;
		$$.src_ip = $3;
		$$.src_port = $4;
		$$.direction = $5;
		$$.dst_ip = $6;
		$$.dst_port = $7;
		TAILQ_INIT(&$$.option_list);
		TAILQ_CONCAT(&$$.option_list, &$9, link);
	}
	;

protocol:
	TOKEN_TCP
	{
		$$ = SNORT_PROTOCOL_TCP;
	}
	| TOKEN_UDP
	{
		$$ = SNORT_PROTOCOL_UDP;
	}
	| TOKEN_IP
	{
		$$ = SNORT_PROTOCOL_IP;
	}
	| TOKEN_ICMP
	{
		$$ = SNORT_PROTOCOL_ICMP;
	}
	| TOKEN_IGMP
	{
		$$ = SNORT_PROTOCOL_IGMP;
	}
	;

action:
	TOKEN_ALERT
	{
		$$ = SNORT_ACTION_ALERT;
	}
	| TOKEN_LOG
	{
		$$ = SNORT_ACTION_LOG;
	}
	| TOKEN_PASS
	{
		$$ = SNORT_ACTION_PASS;
	}
	| TOKEN_REJECT
	{
		$$ = SNORT_ACTION_REJECT;
	}
	| TOKEN_DROP
	{
		$$ = SNORT_ACTION_DROP;
	}
	| TOKEN_DYNAMIC
	{
		$$ = SNORT_ACTION_DYNAMIC;
	}
	| TOKEN_ACTIVATE
	{
		$$ = SNORT_ACTION_ACTIVATE;
	}
	| TOKEN_SDROP
	{
		$$ = SNORT_ACTION_SDROP;
	}
	;

src_ip:
	TOKEN_ANY
	{
		$$.negative = 0;
		$$.low_ip = 0;
		$$.high_ip = (unsigned int)-1;
	}
	;

dst_ip:
	TOKEN_ANY
	{
		$$.negative = 0;
		$$.low_ip = 0;
		$$.high_ip = (unsigned int)-1;
	}
	;

src_port:
	negative_opt TOKEN_NUMBER max_port_opt
	{
		$$.negative = $1;
		$$.low_port = $2;
		if($3)
			$$.high_port = $3;
		else
			$$.high_port = $2;
	}
	| TOKEN_ANY
	{
		$$.negative = 0;
		$$.low_port = 0;
		$$.high_port = (unsigned short)-1;
	}
	;

dst_port:
	negative_opt TOKEN_NUMBER max_port_opt
	{
		$$.negative = $1;
		$$.low_port = $2;
		if($3)
			$$.high_port = $3;
		else
			$$.high_port = $2;
	}
	| TOKEN_ANY
	{
		$$.negative = 0;
		$$.low_port = 0;
		$$.high_port = (unsigned short)-1;
	}
	;

max_port_opt:
	empty
	{
		$$ = (unsigned short)0;
	}
	| TOKEN_COLON TOKEN_NUMBER
	{
		$$ = $2;
	}
	| TOKEN_COLON
	{
		$$ = (unsigned short)-1;
	}
	;

direction:
	TOKEN_SARROW
	{
		$$ = SNORT_DIRECTION_SINGLE;
	}
	| TOKEN_DARROW
	{
		$$ = SNORT_DIRECTION_DOUBLE;
	}
	;

body:
	empty
	{
		TAILQ_INIT(&$$);
	}
	| body item
	{
		snort_option_t *option;
		option = (snort_option_t*)snort_malloc(sizeof(*option));
		if(!option)
		{
			snort_parser_error("malloc option failed\n");
			assert(0);
		}
		*option = $2;
		TAILQ_INSERT_TAIL(&$$, option, link);
	}
	;

item:
	msg_item
	{
		$$ = $1;
	}
	| content_item
	{
		$$ = $1;
	}
	| nocase_item
	{
		$$ = $1;
	}
	| offset_item
	{
		$$ = $1;
	}
	| depth_item
	{
		$$ = $1;
	}
	| distance_item
	{
		$$ = $1;
	}
	| within_item
	{
		$$ = $1;
	}
	| classtype_item
	{
		$$ = $1;
	}
	| sid_item
	{
		$$ = $1;
	}
	| priority_item
	{
		$$ = $1;
	}
	| dsize_item
	{
		$$ = $1;
	}
	| bytetest_item
	{
		$$ = $1;
	}
	| bytejump_item
	{
		$$ = $1;
	}
	| uricontent_item
	{
		$$ = $1;
	}
	| ipproto_item
	{
		$$ = $1;
	}
	| ttl_item
	{
		$$ = $1;
	}
	| itype_item
	{
		$$ = $1;
	}
	| icode_item
	{
		$$ = $1;
	}
	| icmpseq_item
	{
		$$ = $1;
	}
	;

msg_item:
	TOKEN_MSG TOKEN_COLON TOKEN_STRING TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_MSG;
		$$.msg.msg = $3;
	}
	;

content_item:
	TOKEN_CONTENT TOKEN_COLON negative_opt TOKEN_STRING TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_CONTENT;
		$$.content.negative = $3;
		$$.content.content = $4;
	}
	;

negative_opt:
	empty
	{
		$$ = 0;
	}
	| TOKEN_NEG
	{
		$$ = 1;
	}
	;

nocase_item:
	TOKEN_NOCASE TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_NOCASE;
		$$.nocase.nocase = 1;
	}
	;

offset_item:
	TOKEN_OFFSET TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_OFFSET;
		$$.offset.offset = $3;
	}
	;

depth_item:
	TOKEN_DEPTH TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_DEPTH;
		$$.depth.depth = $3;
	}
	;

within_item:
	TOKEN_WITHIN TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_WITHIN;
		$$.within.within = $3;
	}
	;

distance_item:
	TOKEN_DISTANCE TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_DISTANCE;
		$$.distance.distance = $3;
	}
	;

classtype_item:
	TOKEN_CLASSTYPE TOKEN_COLON classtype TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_CLASSTYPE;
		$$.classtype.classtype = $3;
	}
	;

classtype:
	TOKEN_ACCESS
	{
		$$ = SNORT_CLASSTYPE_ACCESS;
	}
	| TOKEN_MISC
	{
		$$ = SNORT_CLASSTYPE_MISC;
	}
	| TOKEN_WEB
	{
		$$ = SNORT_CLASSTYPE_WEB;
	}
	| TOKEN_DDOS
	{
		$$ = SNORT_CLASSTYPE_DDOS;
	}
	| TOKEN_BUFFER
	{
		$$ = SNORT_CLASSTYPE_BUFFER;
	}
	| TOKEN_MAIL
	{
		$$ = SNORT_CLASSTYPE_MAIL;
	}
	| TOKEN_SCAN
	{
		$$ = SNORT_CLASSTYPE_SCAN;
	}
	| TOKEN_BACKDOOR
	{
		$$ = SNORT_CLASSTYPE_BACKDOOR;
	}
	| TOKEN_VIRUS
	{
		$$ = SNORT_CLASSTYPE_VIRUS;
	}
	;

sid_item:
	TOKEN_SID TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_SID;
		$$.sid.sid = $3;
	}
	;

priority_item:
	TOKEN_PRIORITY TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_PRIORITY;
		$$.priority.priority = $3;
	}
	;

uricontent_item:
	TOKEN_URICONTENT TOKEN_COLON negative_opt TOKEN_STRING TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_URICONTENT;
		$$.uricontent.negative = $3;
		$$.uricontent.uricontent = $4;
	}
	;

dsize_item:
	TOKEN_DSIZE TOKEN_COLON dsize_operator_opt TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_DSIZE;
		$$.dsize.operator = $3;
		$$.dsize.dsize = $4;
	}
	;

dsize_operator_opt:
	empty
	{
		$$ = SNORT_OPERATOR_EQ;
	}
	| TOKEN_LT
	{
		$$ = SNORT_OPERATOR_LT;
	}
	| TOKEN_GT
	{
		$$ = SNORT_OPERATOR_GT;
	}
	;

ipproto_item:
	TOKEN_IPPROTO TOKEN_COLON protocol TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_IPPROTO;
		$$.ipproto.ipproto = $3;
	}
	;

ttl_item:
	TOKEN_TTL TOKEN_COLON ttl_operator_opt TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_TTL;
		$$.ttl.operator = $3;
		$$.ttl.ttl = $4;
	}
	;

ttl_operator_opt:
	empty
	{
		$$ = SNORT_OPERATOR_EQ;
	}
	| TOKEN_LT
	{
		$$ = SNORT_OPERATOR_LT;
	}
	| TOKEN_GT
	{
		$$ = SNORT_OPERATOR_GT;
	}
	| TOKEN_LTEQ
	{
		$$ = SNORT_OPERATOR_LTEQ;
	}
	| TOKEN_GTEQ
	{
		$$ = SNORT_OPERATOR_GTEQ;
	}
	| TOKEN_EQ
	{
		$$ = SNORT_OPERATOR_EQ;
	}
	;

itype_item:
	TOKEN_ITYPE TOKEN_COLON itype_operator_opt TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_ICODE;
		$$.icode.operator = $3;
		$$.icode.icode = $4;
	}
	;

itype_operator_opt:
	empty
	{
		$$ = SNORT_OPERATOR_EQ;
	}
	| TOKEN_LT
	{
		$$ = SNORT_OPERATOR_LT;
	}
	| TOKEN_GT
	{
		$$ = SNORT_OPERATOR_GT;
	}
	;

icode_item:
	TOKEN_ICODE TOKEN_COLON icode_operator_opt TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_ITYPE;
		$$.itype.operator = $3;
		$$.itype.itype = $4;
	}
	;

icode_operator_opt:
	empty
	{
		$$ = SNORT_OPERATOR_EQ;
	}
	| TOKEN_LT
	{
		$$ = SNORT_OPERATOR_LT;
	}
	| TOKEN_GT
	{
		$$ = SNORT_OPERATOR_GT;
	}
	;

icmpseq_item:
	TOKEN_ICMPSEQ TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_ICMPSEQ;
		$$.icmpseq.icmpseq = $3;
	}
	;

bytetest_item:
	TOKEN_BYTETEST TOKEN_COLON 
	TOKEN_NUMBER	/*bytes*/
	TOKEN_COMMA negative_opt bytetest_operator 
	TOKEN_COMMA TOKEN_NUMBER	/*value*/
	TOKEN_COMMA TOKEN_NUMBER	/*offset*/
	bytetest_option_list
	TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_BYTETEST;
		$$.bytetest.bytes = $3;
		$$.bytetest.negative = $5;
		$$.bytetest.operator = $6;
		$$.bytetest.value = $8;
		$$.bytetest.offset= $10;
		$$.bytetest.relative = $11.relative;
		$$.bytetest.endian = $11.endian;
		$$.bytetest.string = $11.string;
		$$.bytetest.mask = $11.mask;
	}
	;

bytetest_option_list:
	empty
	{
		$$.mask = 0;
		$$.relative = 0;
		$$.endian = SNORT_ENDIAN_BIG;
		$$.string = SNORT_STRING_DEC;
	}
	| bytetest_option_list TOKEN_COMMA bytetest_option
	{
		$$.mask |= $3.mask;
		if($3.mask==DO_RELATIVE)
			$$.relative = $3.relative;
		if($3.mask==DO_ENDIAN)
			$$.endian = $3.endian;
		if($3.mask==DO_STRING)
			$$.string = $3.string;
	}
	;

bytetest_option:
	bytetest_option_relative
	{
		$$ = $1;
		$$.mask = DO_RELATIVE;
	}
	| bytetest_option_endian
	{
		$$ = $1;
		$$.mask = DO_ENDIAN;
	}
	| bytetest_option_string
	{
		$$ = $1;
		$$.mask = DO_STRING;
	}
	;

bytetest_operator:
	TOKEN_LT
	{
		$$ = SNORT_OPERATOR_LT;
	}
	| TOKEN_GT
	{
		$$ = SNORT_OPERATOR_GT;
	}
	| TOKEN_EQ
	{
		$$ = SNORT_OPERATOR_EQ;
	}
	| TOKEN_BITAND
	{
		$$ = SNORT_OPERATOR_BITAND;
	}
	| TOKEN_BITOR
	{
		$$ = SNORT_OPERATOR_BITOR;
	}
	;

bytetest_option_relative:
	TOKEN_RELATIVE
	{
		$$.relative = 1;
	}
	;

bytetest_option_endian:
	TOKEN_BIG
	{
		$$.endian = SNORT_ENDIAN_BIG;
	}
	| TOKEN_LITTLE
	{
		$$.endian = SNORT_ENDIAN_LITTLE;
	}
	;

bytetest_option_string:
	TOKEN_BYTE_STRING TOKEN_COMMA TOKEN_DEC
	{
		$$.string = SNORT_STRING_DEC;
	}
	| TOKEN_BYTE_STRING TOKEN_COMMA TOKEN_HEX
	{
		$$.string = SNORT_STRING_HEX;
	}
	| TOKEN_BYTE_STRING TOKEN_COMMA TOKEN_OCT
	{
		$$.string = SNORT_STRING_OCT;
	}
	;

bytejump_item:
	TOKEN_BYTEJUMP TOKEN_COLON
	TOKEN_NUMBER
	TOKEN_COMMA TOKEN_NUMBER
	bytejump_option_list
	TOKEN_SEMI
	{
		$$.type = SNORT_OPTION_TYPE_BYTEJUMP;
		$$.bytejump.bytes = $3;
		$$.bytejump.offset = $5;
		$$.bytejump.relative = $6.relative;
		$$.bytejump.endian = $6.endian;
		$$.bytejump.multiplier = $6.multiplier;
		$$.bytejump.align = $6.align;
		$$.bytejump.frombegin = $6.frombegin;
		$$.bytejump.postoffset = $6.postoffset;
		$$.bytejump.mask |= $6.mask;
	}
	;

bytejump_option_list:
	empty
	{
		$$.mask = 0;
		$$.relative = 0;
		$$.endian = SNORT_ENDIAN_BIG;
		$$.multiplier = 1;
		$$.align = 0;
		$$.frombegin = 0;
		$$.postoffset = 0;
	}
	| bytejump_option_list TOKEN_COMMA bytejump_option
	{
		$$.mask |= $3.mask;
		if($3.mask == DO_RELATIVE)
			$$.relative = $3.relative;
		if($3.mask == DO_ENDIAN)
			$$.endian = $3.endian;
		if($3.mask == DO_MULTIPLIER)
			$$.multiplier = $3.multiplier;
		if($3.mask == DO_ALIGN)
			$$.align = $3.align;
		if($3.mask == DO_FROMBEGIN)
			$$.frombegin = $3.frombegin;
		if($3.mask == DO_POSTOFFSET)
			$$.postoffset = $3.postoffset;
	}
	;

bytejump_option:
	bytejump_option_relative
	{
		$$ = $1;
		$$.mask = DO_RELATIVE;
	}
	| bytejump_option_endian
	{
		$$ = $1;
		$$.mask = DO_ENDIAN;
	}
	| bytejump_option_multiplier
	{
		$$ = $1;
		$$.mask = DO_MULTIPLIER;
	}
	| bytejump_option_align
	{
		$$ = $1;
		$$.mask = DO_ALIGN;
	}
	| bytejump_option_frombegin
	{
		$$ = $1;
		$$.mask = DO_FROMBEGIN;
	}
	| bytejump_option_postoffset
	{
		$$ = $1;
		$$.mask = DO_POSTOFFSET;
	}
	;

bytejump_option_relative:
	TOKEN_RELATIVE
	{
		$$.relative = 1;
	}
	;

bytejump_option_endian:
	TOKEN_BIG
	{
		$$.endian = SNORT_ENDIAN_BIG;
	}
	| TOKEN_LITTLE
	{
		$$.endian = SNORT_ENDIAN_LITTLE;
	}
	;

bytejump_option_multiplier:
	TOKEN_MULTIPLIER TOKEN_NUMBER
	{
		$$.multiplier = $2;
	}
	;

bytejump_option_align:
	TOKEN_ALIGN
	{
		$$.align = 1;
	}
	;

bytejump_option_frombegin:
	TOKEN_FROMBEGIN
	{
		$$.frombegin = 1;
	}
	;

bytejump_option_postoffset:
	TOKEN_POSTOFFSET TOKEN_NUMBER
	{
		$$.postoffset = $2;
	}
	;
%%
int snort_error(SNORT_LTYPE *loc, const char *format, ...)
{
	char error_buf[4096];
	va_list ap;

	va_start(ap, format);
	vsnprintf(error_buf, sizeof(error_buf)-1, format, ap);
	error_buf[sizeof(error_buf)-1] = '\0';
	va_end(ap);

	return snort_parser_error("Error: %s\n", error_buf);
}
