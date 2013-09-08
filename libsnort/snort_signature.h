#ifndef SNORT_SIGNATURE_H
#define SNORT_SIGNATURE_H 1

#include "ey_queue.h"

typedef enum snort_action
{
	SNORT_ACTION_ALERT=1,
	SNORT_ACTION_PASS,
	SNORT_ACTION_DROP,
	SNORT_ACTION_SDROP,
	SNORT_ACTION_REJECT,
	SNORT_ACTION_LOG,
	SNORT_ACTION_DYNAMIC,
	SNORT_ACTION_ACTIVATE
}snort_action_t;

typedef enum snort_protocol
{
	SNORT_PROTOCOL_IP=1,
	SNORT_PROTOCOL_TCP,
	SNORT_PROTOCOL_UDP,
	SNORT_PROTOCOL_ICMP,
	SNORT_PROTOCOL_IGMP
}snort_protocol_t;

typedef struct snort_ip
{
	int negative;
	unsigned int low_ip;
	unsigned int high_ip;
}snort_ip_t;

typedef struct snort_port
{
	int negative;
	unsigned short low_port;
	unsigned short high_port;
}snort_port_t;

typedef enum snort_direction
{
	SNORT_DIRECTION_SINGLE=1,
	SNORT_DIRECTION_DOUBLE
}snort_direction_t;

typedef enum snort_option_type
{
	SNORT_OPTION_TYPE_MSG=1,
	SNORT_OPTION_TYPE_CONTENT,
	SNORT_OPTION_TYPE_NOCASE,
	SNORT_OPTION_TYPE_OFFSET,
	SNORT_OPTION_TYPE_DEPTH,
	SNORT_OPTION_TYPE_WITHIN,
	SNORT_OPTION_TYPE_DISTANCE,
	SNORT_OPTION_TYPE_CLASSTYPE,
	SNORT_OPTION_TYPE_SID,
	SNORT_OPTION_TYPE_PRIORITY,
	SNORT_OPTION_TYPE_DSIZE,
	SNORT_OPTION_TYPE_BYTETEST,
	SNORT_OPTION_TYPE_BYTEJUMP,
	SNORT_OPTION_TYPE_URICONTENT,
	SNORT_OPTION_TYPE_IPPROTO,
	SNORT_OPTION_TYPE_TTL,
	SNORT_OPTION_TYPE_ITYPE,
	SNORT_OPTION_TYPE_ICODE,
	SNORT_OPTION_TYPE_ICMPSEQ
}snort_option_type_t;

typedef struct snort_option_msg
{
	char *msg;
}snort_option_msg_t;

typedef struct snort_option_content
{
	char *content;
	int negative;
}snort_option_content_t;

typedef struct snort_option_uricontent
{
	char *uricontent;
	int negative;
}snort_option_uricontent_t;

typedef struct snort_option_nocase
{
	int nocase;
}snort_option_nocase_t;

typedef struct snort_option_offset
{
	int offset;
}snort_option_offset_t;

typedef struct snort_option_depth
{
	int depth;
}snort_option_depth_t;

typedef struct snort_option_within
{
	int within;
}snort_option_within_t;

typedef struct snort_option_distance
{
	int distance;
}snort_option_distance_t;

typedef enum snort_classtype
{
	SNORT_CLASSTYPE_ACCESS=1,
	SNORT_CLASSTYPE_MISC,
	SNORT_CLASSTYPE_WEB,
	SNORT_CLASSTYPE_DDOS,
	SNORT_CLASSTYPE_BUFFER,
	SNORT_CLASSTYPE_MAIL,
	SNORT_CLASSTYPE_SCAN,
	SNORT_CLASSTYPE_BACKDOOR,
	SNORT_CLASSTYPE_VIRUS
}snort_classtype_t;

typedef struct snort_option_classtype
{
	snort_classtype_t classtype;
}snort_option_classtype_t;

typedef struct snort_option_sid
{
	int sid;
}snort_option_sid_t;

typedef struct snort_option_priority
{
	int priority;
}snort_option_priority_t;

typedef enum snort_operator
{
	SNORT_OPERATOR_LT=1,
	SNORT_OPERATOR_GT,
	SNORT_OPERATOR_EQ,
	SNORT_OPERATOR_LTEQ,
	SNORT_OPERATOR_GTEQ,
	SNORT_OPERATOR_BITAND,
	SNORT_OPERATOR_BITOR,
}snort_operator_t;

typedef struct snort_option_dsize
{
	snort_operator_t operator;
	int dsize;
}snort_option_dsize_t;

typedef struct snort_option_ipproto
{
	snort_protocol_t ipproto;
}snort_option_ipproto_t;

typedef struct snort_option_ttl
{
	snort_operator_t operator;
	int ttl;
}snort_option_ttl_t;

typedef struct snort_option_itype
{
	snort_operator_t operator;
	int itype;
}snort_option_itype_t;

typedef struct snort_option_icode
{
	snort_operator_t operator;
	int icode;
}snort_option_icode_t;

typedef struct snort_option_icmpseq
{
	int icmpseq;
}snort_option_icmpseq_t;

typedef enum snort_string_type
{
	SNORT_STRING_DEC=1,
	SNORT_STRING_HEX,
	SNORT_STRING_OCT,
}snort_string_type_t;

typedef enum snort_endian
{
	SNORT_ENDIAN_BIG=1,
	SNORT_ENDIAN_LITTLE,
}snort_endian_t;

#define DO_RELATIVE 	0x01
#define DO_ENDIAN 		0x02
#define DO_STRING 		0x04
#define DO_MULTIPLIER	0x08
#define DO_ALIGN		0x10
#define DO_FROMBEGIN	0x20
#define DO_POSTOFFSET	0x40
typedef struct snort_option_bytetest
{
	int mask;
	int bytes;
	int negative;
	snort_operator_t operator;
	unsigned int value;
	int offset;
	int relative;
	snort_endian_t endian;
	snort_string_type_t string;
}snort_option_bytetest_t;

typedef struct snort_option_bytejump
{
	int mask;
	int bytes;
	int offset;
	int relative;
	snort_endian_t endian;
	int multiplier;
	int align;
	int frombegin;
	int postoffset;
}snort_option_bytejump_t;

typedef struct snort_option
{
	TAILQ_ENTRY(snort_option) link;
	snort_option_type_t type;

	union
	{
		snort_option_msg_t msg;
		snort_option_content_t content;
		snort_option_uricontent_t uricontent;
		snort_option_nocase_t nocase;
		snort_option_offset_t offset;
		snort_option_depth_t depth;
		snort_option_distance_t distance;
		snort_option_within_t within;
		snort_option_classtype_t classtype;
		snort_option_sid_t sid;
		snort_option_priority_t priority;
		snort_option_dsize_t dsize;
		snort_option_ipproto_t ipproto;
		snort_option_ttl_t ttl;
		snort_option_itype_t itype;
		snort_option_icode_t icode;
		snort_option_icmpseq_t icmpseq;
		snort_option_bytetest_t bytetest;
		snort_option_bytejump_t bytejump;
	};
}snort_option_t;
typedef TAILQ_HEAD(snort_option_list, snort_option) snort_option_list_t;

typedef struct snort_signature
{
	TAILQ_ENTRY(snort_signature) link;
	snort_action_t action;
	snort_protocol_t protocol;
	snort_ip_t src_ip;
	snort_port_t src_port;
	snort_direction_t direction;
	snort_ip_t dst_ip;
	snort_port_t dst_port;
	snort_option_list_t option_list;
}snort_signature_t;
typedef TAILQ_HEAD(snort_signature_list, snort_signature) snort_signature_list_t;
#endif
