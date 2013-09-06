%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "snort_info.h"
#include "snort_parser.h"

#define YY_USER_ACTION										\
	{														\
		yylloc->first_line = yylineno;						\
		yylloc->first_column = yycolumn;					\
		yylloc->last_line = yylineno;						\
		yylloc->last_column = yycolumn+yyleng;				\
		yycolumn += yyleng;									\
	}

#define RETURN(sym)											\
	do														\
	{														\
		char *ret_str = print_buffer();						\
		snort_lexer_debug("%d-%d:\n", yylineno, yycolumn);	\
		if(!ret_str || !ret_str[0])							\
			ret_str = yytext;								\
		snort_lexer_debug("line %d returns %s, value: %s\n",\
			__LINE__, #sym, ret_str);						\
		reset_buffer();										\
		return sym;											\
	}while(0)

#define GOTO(s)												\
	do														\
	{														\
		snort_lexer_debug("%d-%d:\n", yylineno, yycolumn);	\
		snort_lexer_debug("line %d goto state %s(%d)\n",	\
			__LINE__, #s, s);								\
		BEGIN s;											\
	}while(0)


#define DEFAULT_BUFFER_LEN	4096
static char *buffer;
static int buffer_size;
static int buffer_len;

static void reset_buffer();
static char *print_buffer();
static void add_buffer(char *str, int len);
%}
%option debug
%option nodefault
%option noinput
%option nounput
%option noyywrap
%option never-interactive
%option header-file="snort_lexer.h"
%option outfile="snort_lexer.c"
%option prefix="snort_"
%option bison-bridge
%option bison-locations
%option reentrant
%option ansi-definitions
%option ansi-prototypes
%option yylineno
%option case-insensitive

%x QUOTE_STRING

space	[ \t\v\f\n\r]
digit	[0-9]
hex		[0-9a-f]
dec_number [\-]?{digit}+
hex_number 0x{hex}+
%%
<INITIAL>
{
	/*keywords*/
	"alert"				{RETURN(TOKEN_ALERT);}
	"log"				{RETURN(TOKEN_LOG);}
	"drop"				{RETURN(TOKEN_DROP);}
	"reject"			{RETURN(TOKEN_REJECT);}
	"sdrop"				{RETURN(TOKEN_SDROP);}
	"pass"				{RETURN(TOKEN_PASS);}
	"activate"			{RETURN(TOKEN_ACTIVATE);}
	"dynamic"			{RETURN(TOKEN_DYNAMIC);}
	"tcp"				{RETURN(TOKEN_TCP);}
	"udp"				{RETURN(TOKEN_UDP);}
	"icmp"				{RETURN(TOKEN_ICMP);}
	"ip"				{RETURN(TOKEN_IP);}
	"any"				{RETURN(TOKEN_ANY);}
	"->"				{RETURN(TOKEN_SARROW);}
	"<>"				{RETURN(TOKEN_DARROW);}
	{dec_number}		{RETURN(TOKEN_NUMBER);}
	{hex_number}		{RETURN(TOKEN_NUMBER);}
	":"					{RETURN(TOKEN_COLON);}
	"("					{RETURN(TOKEN_LPAREN);}
	")"					{RETURN(TOKEN_RPAREN);}
	"msg"				{RETURN(TOKEN_MSG);}
	";"					{RETURN(TOKEN_SEMI);}
	"!"					{RETURN(TOKEN_NEG);}
	","					{RETURN(TOKEN_COMMA);}
	"nocase"			{RETURN(TOKEN_NOCASE);}
	"offset"			{RETURN(TOKEN_OFFSET);}
	"depth"				{RETURN(TOKEN_DEPTH);}
	"distance"			{RETURN(TOKEN_DISTANCE);}
	"within"			{RETURN(TOKEN_WITHIN);}
	"classtype"			{RETURN(TOKEN_CLASSTYPE);}
	"sid"				{RETURN(TOKEN_SID);}
	"priority"			{RETURN(TOKEN_PRIORITY);}
	"content"			{RETURN(TOKEN_CONTENT);}
	"uricontent"		{RETURN(TOKEN_URICONTENT);}
	"dsize"				{RETURN(TOKEN_DSIZE);}
	"<"					{RETURN(TOKEN_LT);}
	">"					{RETURN(TOKEN_GT);}
	"ip_proto"			{RETURN(TOKEN_IPPROTO);}
	"igmp"				{RETURN(TOKEN_IGMP);}
	"ttl"				{RETURN(TOKEN_TTL);}
	"<="				{RETURN(TOKEN_LTEQ);}
	">="				{RETURN(TOKEN_GTEQ);}
	"="					{RETURN(TOKEN_EQ);}
	"itype"				{RETURN(TOKEN_ITYPE);}
	"icode"				{RETURN(TOKEN_ICODE);}
	"icmp_seq"			{RETURN(TOKEN_ICMPSEQ);}
	"byte_test"			{RETURN(TOKEN_BYTETEST);}
	"&"					{RETURN(TOKEN_BITAND);}
	"^"					{RETURN(TOKEN_BITOR);}
	"relative"			{RETURN(TOKEN_RELATIVE);}
	"big"				{RETURN(TOKEN_BIG);}
	"little"			{RETURN(TOKEN_LITTLE);}
	"string"			{RETURN(TOKEN_BYTE_STRING);}
	"dec"				{RETURN(TOKEN_DEC);}
	"hex"				{RETURN(TOKEN_HEX);}
	"oct"				{RETURN(TOKEN_OCT);}
	"byte_jump"			{RETURN(TOKEN_BYTEJUMP);}
	"multiplier"		{RETURN(TOKEN_MULTIPLIER);}
	"align"				{RETURN(TOKEN_ALIGN);}
	"from_beginning"	{RETURN(TOKEN_FROMBEGIN);}
	"post_offset"		{RETURN(TOKEN_POSTOFFSET);}
	"Access Control"	{RETURN(TOKEN_ACCESS);}
	"Misc"				{RETURN(TOKEN_MISC);}
	"Web Attacks"		{RETURN(TOKEN_WEB);}
	"DoS/DDoS"			{RETURN(TOKEN_DDOS);}
	"Buffer Overflow"	{RETURN(TOKEN_BUFFER);}
	"Mail"				{RETURN(TOKEN_MAIL);}
	"Scan"				{RETURN(TOKEN_SCAN);}
	"Backdoor/Trojan"	{RETURN(TOKEN_BACKDOOR);}
	"Virus/Worm"		{RETURN(TOKEN_VIRUS);}
	"\""				{GOTO(QUOTE_STRING); reset_buffer();}
	{space}				{/*eat space*/}
	.					{snort_lexer_error("bad char %x\n", yytext[0]); return -1;}
}

<QUOTE_STRING>
{
	/*quoted string*/
	"\\\""				{add_buffer(yytext, yyleng);}
	"\\\\"				{add_buffer(yytext, yyleng);}
	"\""				{GOTO(INITIAL); snort_lexer_debug("we get: %s\n", buffer); return TOKEN_STRING;}
	.					{add_buffer(yytext, 1);}
	<<EOF>>				{snort_lexer_error("EOF is not accepted\n"); return -1;}
}
%%

static void reset_buffer()
{
	if(buffer)
		buffer[0] = '\0';
	buffer_len = 0;
}

static void add_buffer(char *str, int len)
{
	assert(str!=NULL);
	assert(len>=0);
	if(buffer_len+len >= buffer_size)
	{
		buffer = (char*)realloc(buffer, buffer_len+len+DEFAULT_BUFFER_LEN);
		assert(buffer != NULL);
		buffer_size = buffer_len+len+DEFAULT_BUFFER_LEN;
	}
	
	memcpy(buffer+buffer_len, str, len);
	buffer[buffer_len+len] = '\0';
	buffer_len += len;
}

static char* print_buffer()
{
	return buffer?buffer:"";
}
