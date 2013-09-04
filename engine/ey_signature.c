#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ey_signature.h"
#include "engine_mem.h"
#include "ey_info.h"


ey_rhs_item_condition_t *alloc_rhs_item_condition(ey_location_t *location, 
	char *code, void *symbol)
{
	assert(location != NULL);

	ey_rhs_item_condition_t *ret = (ey_rhs_item_condition_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->raw_code = code;
	ret->symbol = symbol;
	return ret;
}

void free_rhs_item_condition(ey_rhs_item_condition_t *condition)
{
	if(!condition)
		return;
	if(condition->raw_code)
		parser_free(condition->raw_code);
	parser_free(condition);
}

ey_rhs_item_action_t *alloc_rhs_item_action(ey_location_t *location, 
	char *code, void *symbol)
{
	assert(location != NULL);

	ey_rhs_item_action_t *ret = (ey_rhs_item_action_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->raw_code = code;
	ret->symbol = symbol;
	return ret;
}

void free_rhs_item_action(ey_rhs_item_action_t *action)
{
	if(!action)
		return;
	if(action->raw_code)
		parser_free(action->raw_code);
	parser_free(action);
}

ey_rhs_item_t *alloc_rhs_item(ey_location_t *location, 
	ey_rhs_item_condition_t *condition, 
	ey_rhs_item_action_t *action)
{
	assert(location != NULL);
	ey_rhs_item_t *ret = (ey_rhs_item_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->condition = condition;
	ret->action = action;
	return ret;
}

void free_rhs_item(ey_rhs_item_t *item)
{
	if(!item)
		return;
	if(item->condition)
		free_rhs_item_condition(item->condition);
	if(item->action)
		free_rhs_item_action(item->action);
	parser_free(item);
}

ey_rhs_signature_t *alloc_rhs_signature(ey_location_t *location, 
	ey_rhs_item_list_t *rhs_list)
{
	assert(location != NULL);
	assert(rhs_list != NULL);
	ey_rhs_signature_t *ret = (ey_rhs_signature_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	TAILQ_INIT(&ret->rhs_item_list);
	TAILQ_CONCAT(&ret->rhs_item_list, rhs_list, link);
	return ret;
}

void free_rhs_signature(ey_rhs_signature_t *rhs_signature)
{
	if(!rhs_signature)
		return;
	ey_rhs_item_t *item=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(item, &rhs_signature->rhs_item_list, link, tmp)
	{
		free_rhs_item(item);
	}
}

ey_signature_t *alloc_signature(unsigned long id,
	ey_location_t *location, ey_rhs_signature_list_t *signature_list)
{
	assert(location != NULL);
	assert(signature_list != NULL);
	ey_signature_t *ret = (ey_signature_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	TAILQ_INIT(&ret->rhs_signature_list);
	TAILQ_CONCAT(&ret->rhs_signature_list, signature_list, link);
	return ret;
}

void free_signature(ey_signature_t *signature)
{
	if(!signature)
		return;
	ey_rhs_signature_t *item = NULL, *tmp = NULL;
	TAILQ_FOREACH_SAFE(item, &signature->rhs_signature_list, link, tmp)
	{
		free_rhs_signature(item);
	}
}
