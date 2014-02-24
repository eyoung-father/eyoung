#ifndef HTML_TYPE_H
#define HTML_TYPE_H 1

#include "ey_queue.h"
#include "ey_string.h"
#include "html_decode.h"

typedef ey_string_t html_string_t;
/*
 * HTML DOM NODE PROT
 * */
typedef struct html_node_prot
{
	int type;
	html_string_t value;
	TAILQ_ENTRY(html_node_prot) next;
}html_node_prot_t;
typedef TAILQ_HEAD(html_node_prot_list, html_node_prot) html_node_prot_list_t;

/*
 * HTML DOM NODE
 * */
struct html_node;
typedef TAILQ_HEAD(html_node_list, html_node) html_node_list_t;
typedef struct html_node
{
	int type;
	html_string_t text;				/*for text between two tags*/
	struct html_node *parent;
	html_node_list_t child;
	html_node_prot_list_t prot;		/*for tag*/
	TAILQ_ENTRY(html_node) sib;
}html_node_t;

#define IS_CLOSING_NODE(node)			\
(										\
	node->type!=SYM_TAG_AREA		&&	\
	noode->type!=SYM_TAG_BASE 		&&	\
	node->type!=SYM_TAG_BASEFONT	&&	\
	node->type!=SYM_TAG_BR			&&	\
	node->type!=SYM_TAG_COL			&&	\
	node->type!=SYM_TAG_FRAME		&&	\
	node->type!=SYM_TAG_IMG			&&	\
	node->type!=SYM_TAG_INPUT		&&	\
	node->type!=SYM_TAG_LINK		&&	\
	node->type!=SYM_TAG_META		&&	\
	node->type!=SYM_TAG_PARAM			\
)

#define IS_DETECT_NODE(type)			\
(										\
	type==SYM_TAG_APPLET			||	\
	type==SYM_TAG_FRAME				||	\
	type==SYM_TAG_IFRAME			||	\
	type==SYM_TAG_LINK				||	\
	type==SYM_TAG_OBJECT			||	\
	type==SYM_TAG_SCRIPT				\
)

#define IS_TEXT_NODE(node) (node->type==SYM_TEXT)
#define IS_TAG_NODE(node) (!IS_TEXT_NODE(node))

/*
 * HTML PARSER
 * */
typedef struct html_parser
{
	void *parser;
	void *lexier;

	char *saved;
	int saved_len;
	char last_frag;
	char greedy;
}html_parser_t;

/*
 * HTML PRIVATE DATA
 * */
typedef struct html_data
{
	html_handler_t decoder;
	engine_work_t *engine_work;

	html_node_list_t html_root;
	html_parser_t parser;
	int create_dom;
}html_data_t;

#endif
