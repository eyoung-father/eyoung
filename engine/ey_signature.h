#ifndef EY_SIGNATURE_H
#define EY_SIGNATURE_H 1

#include "ey_queue.h"
#include "ey_loc.h"

struct ey_engine;
typedef struct ey_rhs_item_condition
{
	ey_location_t location;
	char *raw_code;
	void *symbol;
}ey_rhs_item_condition_t;
extern ey_rhs_item_condition_t *ey_alloc_rhs_item_condition(struct ey_engine *eng, ey_location_t *location, 
	char *code, void *symbol);
extern void ey_free_rhs_item_condition(struct ey_engine *eng, ey_rhs_item_condition_t *condition);

typedef struct ey_rhs_item_action
{
	ey_location_t location;
	char *raw_code;
	void *symbol;
}ey_rhs_item_action_t;
extern ey_rhs_item_action_t *ey_alloc_rhs_item_action(struct ey_engine *eng, ey_location_t *location, 
	char *code, void *symbol);
extern void ey_free_rhs_item_action(struct ey_engine *eng, ey_rhs_item_action_t *action);

typedef struct ey_rhs_item
{
	TAILQ_ENTRY(ey_rhs_item) link;
	ey_location_t location;
	char *event_name;
	char *cluster_condition;
	ey_rhs_item_condition_t *condition;
	ey_rhs_item_action_t *action;
}ey_rhs_item_t;
typedef TAILQ_HEAD(ey_rhs_item_list, ey_rhs_item) ey_rhs_item_list_t;
extern ey_rhs_item_t *ey_alloc_rhs_item(struct ey_engine *eng, ey_location_t *location, 
	char *event_name,
	char *cluster_condition,
	ey_rhs_item_condition_t *condition, 
	ey_rhs_item_action_t *action);
extern void ey_free_rhs_item(struct ey_engine *eng, ey_rhs_item_t *item);

typedef struct ey_rhs_signature
{
	ey_location_t location;
	TAILQ_ENTRY(ey_rhs_signature) link;
	ey_rhs_item_list_t rhs_item_list;
}ey_rhs_signature_t;
typedef TAILQ_HEAD(ey_rhs_signature_list, ey_rhs_signature) ey_rhs_signature_list_t;
extern ey_rhs_signature_t *ey_alloc_rhs_signature(struct ey_engine *eng, ey_location_t *location, 
	ey_rhs_item_list_t *rhs_list);
extern void ey_free_rhs_signature(struct ey_engine *eng, ey_rhs_signature_t *rhs_signature);

typedef struct ey_signature
{
	TAILQ_ENTRY(ey_signature) link;
	unsigned long id;
	ey_location_t location;
	ey_rhs_signature_list_t rhs_signature_list;
}ey_signature_t;
typedef TAILQ_HEAD(ey_signature_list, ey_signature) ey_signature_list_t;
extern ey_signature_t *ey_alloc_signature(struct ey_engine *eng, unsigned long id,
	ey_location_t *location, ey_rhs_signature_list_t *signature_list);
extern void ey_free_signature(struct ey_engine *eng, ey_signature_t *signature);

struct ey_event;
typedef struct ey_code
{
	TAILQ_ENTRY(ey_code) link;
	ey_location_t location;
	#define EY_CODE_NORMAL 	1
	#define EY_CODE_IMPORT	2
	#define EY_CODE_EVENT	3
	int type;
	union
	{
		char *raw_code;
		char *filename;
		struct ey_event *event;
	};
}ey_code_t;
typedef TAILQ_HEAD(ey_code_list, ey_code) ey_code_list_t;
extern ey_code_t *ey_alloc_code(struct ey_engine *eng, ey_location_t *location, void *code, int type);
extern void ey_free_code(struct ey_engine *eng, ey_code_t *code);

typedef struct ey_signature_file
{
	char *output_file;
	ey_code_list_t prologue_list;
	ey_signature_list_t signature_list;
	ey_code_t *epilogue;
}ey_signature_file_t;
extern ey_signature_file_t *ey_alloc_signature_file(struct ey_engine *eng, char *output_file,
	ey_code_list_t *prologue_list, 
	ey_signature_list_t *signature_list,
	ey_code_t *epilogue);
extern void ey_free_signature_file(struct ey_engine *eng, ey_signature_file_t *file);

extern int ey_signature_init(struct ey_engine *eng);
extern void ey_signature_finit(struct ey_engine *eng);
#endif
