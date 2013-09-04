#ifndef EY_SIGNATURE_H
#define EY_SIGNATURE_H 1

#include "ey_queue.h"
#include "ey_loc.h"

typedef struct ey_rhs_item_condition
{
	ey_location_t location;
	char *raw_code;
	void *symbol;
}ey_rhs_item_condition_t;
extern ey_rhs_item_condition_t *alloc_rhs_item_condition(ey_location_t *location, 
	char *code, void *symbol);
extern void free_rhs_item_condition(ey_rhs_item_condition_t *condition);

typedef struct ey_rhs_item_action
{
	ey_location_t location;
	char *raw_code;
	void *symbol;
}ey_rhs_item_action_t;
extern ey_rhs_item_action_t *alloc_rhs_item_action(ey_location_t *location, 
	char *code, void *symbol);
extern void free_rhs_item_action(ey_rhs_item_action_t *action);

typedef struct ey_rhs_item
{
	TAILQ_ENTRY(ey_rhs_item) link;
	ey_location_t location;
	ey_rhs_item_condition_t *condition;
	ey_rhs_item_action_t *action;
}ey_rhs_item_t;
typedef TAILQ_HEAD(ey_rhs_item_list, ey_rhs_item) ey_rhs_item_list_t;
extern ey_rhs_item_t *alloc_rhs_item(ey_location_t *location, 
	ey_rhs_item_condition_t *condition, 
	ey_rhs_item_action_t *action);
extern void free_rhs_item(ey_rhs_item_t *item);

typedef struct ey_rhs_signature
{
	ey_location_t location;
	TAILQ_ENTRY(ey_rhs_signature) link;
	ey_rhs_item_list_t rhs_item_list;
}ey_rhs_signature_t;
typedef TAILQ_HEAD(ey_rhs_signature_list, ey_rhs_signature) ey_rhs_signature_list_t;
extern ey_rhs_signature_t *alloc_rhs_signature(ey_location_t *location, 
	ey_rhs_item_list_t *rhs_list);
extern void free_rhs_signature(ey_rhs_signature_t *rhs_signature);

typedef struct ey_signature
{
	unsigned long id;
	ey_location_t location;
	ey_rhs_signature_list_t rhs_signature_list;
}ey_signature_t;
extern ey_signature_t *alloc_signature(unsigned long id,
	ey_location_t *location, ey_rhs_signature_list_t *signature_list);
extern void free_signature(ey_signature_t *signature);
#endif
