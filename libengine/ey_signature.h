#ifndef EY_SIGNATURE_H
#define EY_SIGNATURE_H 1

#include "ey_queue.h"
#include "ey_loc.h"
#include "libengine_type.h"

struct ey_engine;
#define MAX_CONDITION_FUNC_NAME_LEN	128
typedef struct ey_rhs_item_condition
{
	ey_location_t location;
	char *raw_code;
	char *func_name;
	void *addr;
}ey_rhs_item_condition_t;
extern ey_rhs_item_condition_t *ey_alloc_rhs_item_condition(struct ey_engine *eng, ey_location_t *location, 
	char *code, char *func_name, void *addr);
extern void ey_free_rhs_item_condition(struct ey_engine *eng, ey_rhs_item_condition_t *condition);

#define MAX_ACTION_FUNC_NAME_LEN	128
typedef struct ey_rhs_item_action
{
	ey_location_t location;
	char *raw_code;
	char *func_name;
	void *addr;
}ey_rhs_item_action_t;
extern ey_rhs_item_action_t *ey_alloc_rhs_item_action(struct ey_engine *eng, ey_location_t *location, 
	char *code, char *func_name, void *addr);
extern void ey_free_rhs_item_action(struct ey_engine *eng, ey_rhs_item_action_t *action);

typedef struct ey_cluster_condition
{
	char *pattern;
	size_t pattern_len;
	char *preprocessor;
}ey_cluster_condition_t;

typedef struct ey_rhs_item
{
	int clustered;
	unsigned long signature_id;
	unsigned long rhs_id;

	unsigned long rhs_signature_position;
	unsigned long rhs_item_position;
	TAILQ_ENTRY(ey_rhs_item) link;
	TAILQ_ENTRY(ey_rhs_item) event_link;
	ey_location_t location;
	char *event_name;
	ey_cluster_condition_t *cluster_condition;
	ey_rhs_item_condition_t *condition;
	ey_rhs_item_action_t *action;
}ey_rhs_item_t;
typedef TAILQ_HEAD(ey_rhs_item_list, ey_rhs_item) ey_rhs_item_list_t;

extern ey_rhs_item_t *ey_alloc_rhs_item(struct ey_engine *eng, ey_location_t *location, 
	char *event_name,
	ey_cluster_condition_t *cluster_condition,
	ey_rhs_item_condition_t *condition, 
	ey_rhs_item_action_t *action);
extern void ey_free_rhs_item(struct ey_engine *eng, ey_rhs_item_t *item);

typedef struct ey_rhs_signature
{
	unsigned long signature_id;
	unsigned long rhs_signature_position;
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
	unsigned long signature_id;
	TAILQ_ENTRY(ey_signature) link;
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
	#define EY_CODE_NORMAL				1
	#define EY_CODE_IMPORT				2
	#define EY_CODE_EVENT				3
	#define EY_CODE_FILE_INIT			4
	#define EY_CODE_FILE_FINIT			5
	#define EY_CODE_WORK_INIT			6
	#define EY_CODE_WORK_FINIT			7
	#define EY_CODE_EVENT_INIT			8
	#define EY_CODE_EVENT_FINIT			9
	#define EY_CODE_EVENT_PREPROCESSOR	10
	int type;
	union
	{
		char *raw_code;
		char *filename;
		int event;
		struct
		{
			char *function;
			char *event_name;
			void *handle;
		};
	};
}ey_code_t;
typedef TAILQ_HEAD(ey_code_list, ey_code) ey_code_list_t;
extern ey_code_t *ey_alloc_code(struct ey_engine *eng, ey_location_t *location, void *code, void *addr, void *event, int type);
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
extern ey_signature_t *ey_find_signature(struct ey_engine *eng, unsigned long id);
extern int ey_insert_signature(struct ey_engine *eng, ey_signature_t *signature);

extern int ey_signature_add_init(struct ey_engine *eng, const char *function, 
	file_init_handle address, ey_location_t *location);
extern int ey_signature_add_finit(struct ey_engine *eng, const char *function, 
	file_finit_handle address, ey_location_t *location);
#endif
