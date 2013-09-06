%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "snort_info.h"
#include "snort_parser.h"
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
}

%type	<string>			TOKEN_STRING
%type	<number>			TOKEN_NUMBER

%debug
%verbose
%locations
%defines "snort_parser.h"
%output "snort_parser.c"
%define api.prefix snort_
%define api.pure full
%define api.push-pull push

%start file
%%
empty:
	;

file:
	empty
	| signature_list
	;

signature_list:
	signature
	| signature_list signature
	;

signature:
	action protocol src_ip src_port direction dst_ip dst_port TOKEN_LPAREN body TOKEN_RPAREN
	;

protocol:
	TOKEN_TCP
	| TOKEN_UDP
	| TOKEN_IP
	| TOKEN_ICMP
	| TOKEN_IGMP
	;

action:
	TOKEN_ALERT
	| TOKEN_LOG
	| TOKEN_PASS
	| TOKEN_REJECT
	| TOKEN_DROP
	| TOKEN_DYNAMIC
	| TOKEN_ACTIVATE
	| TOKEN_SDROP
	;

src_ip:
	TOKEN_ANY
	;

dst_ip:
	TOKEN_ANY
	;

src_port:
	negative_opt TOKEN_NUMBER max_port_opt
	| TOKEN_ANY
	;

dst_port:
	negative_opt TOKEN_NUMBER max_port_opt
	| TOKEN_ANY
	;

max_port_opt:
	empty
	| TOKEN_COLON TOKEN_NUMBER
	| TOKEN_COLON
	;

direction:
	TOKEN_SARROW
	| TOKEN_DARROW
	;

body:
	item
	| body item
	;

item:
	msg_item
	| content_item
	| nocase_item
	| offset_item
	| depth_item
	| distance_item
	| within_item
	| classtype_item
	| sid_item
	| priority_item
	| dsize_item
	| bytetest_item
	| bytejump_item
	| uricontent_item
	| ipproto_item
	| ttl_item
	| itype_item
	| icode_item
	| icmpseq_item
	;

msg_item:
	TOKEN_MSG TOKEN_COLON TOKEN_STRING TOKEN_SEMI
	;

content_item:
	TOKEN_CONTENT TOKEN_COLON negative_opt TOKEN_STRING TOKEN_SEMI
	;

negative_opt:
	empty
	| TOKEN_NEG
	;

nocase_item:
	TOKEN_NOCASE TOKEN_SEMI
	;

offset_item:
	TOKEN_OFFSET TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

depth_item:
	TOKEN_DEPTH TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

within_item:
	TOKEN_WITHIN TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

distance_item:
	TOKEN_DISTANCE TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

classtype_item:
	TOKEN_CLASSTYPE TOKEN_COLON classtype TOKEN_SEMI
	;

classtype:
	TOKEN_ACCESS
	| TOKEN_MISC
	| TOKEN_WEB
	| TOKEN_DDOS
	| TOKEN_BUFFER
	| TOKEN_MAIL
	| TOKEN_SCAN
	| TOKEN_BACKDOOR
	| TOKEN_VIRUS
	;

sid_item:
	TOKEN_SID TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

priority_item:
	TOKEN_PRIORITY TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

uricontent_item:
	TOKEN_URICONTENT TOKEN_COLON negative_opt TOKEN_STRING TOKEN_SEMI
	;

dsize_item:
	TOKEN_DSIZE TOKEN_COLON dsize_operator_opt TOKEN_NUMBER TOKEN_SEMI
	;

dsize_operator_opt:
	empty
	| TOKEN_LT
	| TOKEN_GT
	;

ipproto_item:
	TOKEN_IPPROTO TOKEN_COLON protocol TOKEN_SEMI
	;

ttl_item:
	TOKEN_TTL TOKEN_COLON ttl_operator_opt TOKEN_NUMBER TOKEN_SEMI
	;

ttl_operator_opt:
	empty
	| TOKEN_LT
	| TOKEN_GT
	| TOKEN_LTEQ
	| TOKEN_GTEQ
	| TOKEN_EQ
	;

itype_item:
	TOKEN_ITYPE TOKEN_COLON itype_operator_opt TOKEN_NUMBER TOKEN_SEMI
	;

itype_operator_opt:
	empty
	| TOKEN_LT
	| TOKEN_GT
	;

icode_item:
	TOKEN_ICODE TOKEN_COLON icode_operator_opt TOKEN_NUMBER TOKEN_SEMI
	;

icode_operator_opt:
	empty
	| TOKEN_LT
	| TOKEN_GT
	;

icmpseq_item:
	TOKEN_ICMPSEQ TOKEN_COLON TOKEN_NUMBER TOKEN_SEMI
	;

bytetest_item:
	TOKEN_BYTETEST TOKEN_COLON 
	TOKEN_NUMBER	/*bytes*/
	TOKEN_COMMA negative_opt bytetest_operator 
	TOKEN_COMMA TOKEN_NUMBER	/*value*/
	TOKEN_COMMA TOKEN_NUMBER	/*offset*/
	bytetest_option_list
	TOKEN_SEMI
	;

bytetest_option_list:
	empty
	| bytetest_option_list TOKEN_COMMA bytetest_option
	;

bytetest_option:
	bytetest_option_relative
	| bytetest_option_endian
	| bytetest_option_string
	;

bytetest_operator:
	TOKEN_LT
	| TOKEN_GT
	| TOKEN_EQ
	| TOKEN_BITAND
	| TOKEN_BITOR
	;

bytetest_option_relative:
	TOKEN_RELATIVE
	;

bytetest_option_endian:
	TOKEN_BIG
	| TOKEN_LITTLE
	;

bytetest_option_string:
	TOKEN_BYTE_STRING TOKEN_COMMA TOKEN_DEC
	| TOKEN_BYTE_STRING TOKEN_COMMA TOKEN_HEX
	| TOKEN_BYTE_STRING TOKEN_COMMA TOKEN_OCT
	;

bytejump_item:
	TOKEN_BYTEJUMP TOKEN_COLON
	TOKEN_NUMBER
	TOKEN_COMMA TOKEN_NUMBER
	bytejump_option_list
	TOKEN_SEMI
	;

bytejump_option_list:
	empty
	| bytejump_option_list TOKEN_COMMA bytejump_option
	;

bytejump_option:
	bytejump_option_relative
	| bytejump_option_endian
	| bytejump_option_multiplier
	| bytejump_option_align
	| bytejump_option_frombegin
	| bytejump_option_postoffset
	;

bytejump_option_relative:
	TOKEN_RELATIVE
	;

bytejump_option_endian:
	TOKEN_BIG
	| TOKEN_LITTLE
	;

bytejump_option_multiplier:
	TOKEN_MULTIPLIER TOKEN_NUMBER
	;

bytejump_option_align:
	TOKEN_ALIGN
	;

bytejump_option_frombegin:
	TOKEN_FROMBEGIN
	;

bytejump_option_postoffset:
	TOKEN_POSTOFFSET TOKEN_NUMBER
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
