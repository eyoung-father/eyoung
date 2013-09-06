%{
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "snort_info.h"
#include "snort_parser.h"

#define DEFAULT_BUFFER_LEN	4096
static char *buffer;
static int buffer_size;
static int buffer_len;

static void reset_buffer();
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

%x QUOTE_STRING

space	[ \t\v\f\n]
digit	[0-9]
hex		[0-9a-f]
dec_number [\-]?{digit}+
hex_number 0x{hex}+
%%
<INITIAL>
{
	/*keywords*/
	"alert"				{return TOKEN_ALERT;}
	"any"				{return TOKEN_ANY;}
	"->"				{return TOKEN_SARROW;}
	"<>"				{return TOKEN_DARROW;}
	{dec_number}		{return TOKEN_NUMBER;}
	{hex_number}		{return TOKEN_NUMBER;}
	":"					{return TOKEN_COLON;}
	"("					{return TOKEN_LPAREN;}
	")"					{return TOKEN_RPAREN;}
	"msg"				{return TOKEN_MSG;}
	";"					{return TOKEN_SEMI;}
	"!"					{return TOKEN_NEG;}
	","					{return TOKEN_COMMA;}
	"nocase"			{return TOKEN_NOCASE;}
	"offset"			{return TOKEN_OFFSET;}
	"depth"				{return TOKEN_DEPTH;}
	"distance"			{return TOKEN_DISTANCE;}
	"within"			{return TOKEN_WITHIN;}
	"classtype"			{return TOKEN_CLASSTYPE;}
	"sid"				{return TOKEN_SID;}
	"priority"			{return TOKEN_PRIORITY;}
	"uricontent"		{return TOKEN_URICONTENT;}
	"dsize"				{return TOKEN_DSIZE;}
	"<"					{return TOKEN_LT;}
	">"					{return TOKEN_GT;}
	"ip_proto"			{return TOKEN_IPPROTO;}
	"ttl"				{return TOKEN_TTL;}
	"<="				{return TOKEN_LTEQ;}
	">="				{return TOKEN_GTEQ;}
	"="					{return TOKEN_EQ;}
	"itype"				{return TOKEN_ITYPE;}
	"icode"				{return TOKEN_ICODE;}
	"icmp_seq"			{return TOKEN_ICMPSEQ;}
	"byte_test"			{return TOKEN_BYTETEST;}
	"&"					{return TOKEN_BITAND;}
	"^"					{return TOKEN_BITOR;}
	"relative"			{return TOKEN_RELATIVE;}
	"big"				{return TOKEN_BIG;}
	"little"			{return TOKEN_LITTLE;}
	"dec"				{return TOKEN_DEC;}
	"hex"				{return TOKEN_HEX;}
	"oct"				{return TOKEN_OCT;}
	"byte_jump"			{return TOKEN_BYTEJUMP;}
	"multiplier"		{return TOKEN_MULTIPLIER;}
	"align"				{return TOKEN_ALIGN;}
	"from_beginning"	{return TOKEN_FROMBEGIN;}
	"post_offset"		{return TOKEN_POSTOFFSET;}
	"Access Control"	{return TOKEN_ACCESS;}
	"Misc"				{return TOKEN_MISC;}
	"Web Attack"		{return TOKEN_WEB;}
	"DoS/DDoS"			{return TOKEN_DDOS;}
	"Buffer Overflow"	{return TOKEN_BUFFER;}
	"Mail"				{return TOKEN_MAIL;}
	"Scan"				{return TOKEN_SCAN;}
	"Backdoor/Trojan"	{return TOKEN_BACKDOOR;}
	"Virus/Worm"		{return TOKEN_VIRUS;}
	"\""				{BEGIN QUOTE_STRING; reset_buffer();}
	{space}				{/*eat space*/}
	.					{snort_lexer_error("bad char\n"); return -1;}
}

<QUOTE_STRING>
{
	/*quoted string*/
	"\\\""				{add_buffer(yytext, yyleng);}
	"\""				{BEGIN INITIAL; snort_lexer_debug("we get: %s\n", buffer); return TOKEN_STRING;}
	.					{add_buffer(yytext, 1);}
	<<EOF>>				{snort_lexer_error("EOF is not accepted\n"); return -1;}
}
%%

static void reset_buffer()
{
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
