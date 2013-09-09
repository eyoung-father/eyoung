#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ey_signature.h"
#include "engine_mem.h"
#include "ey_info.h"


ey_rhs_item_condition_t *ey_alloc_rhs_item_condition(ey_location_t *location, 
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

void ey_free_rhs_item_condition(ey_rhs_item_condition_t *condition)
{
	if(!condition)
		return;
	if(condition->raw_code)
		lexer_free(condition->raw_code);
	parser_free(condition);
}

ey_rhs_item_action_t *ey_alloc_rhs_item_action(ey_location_t *location, 
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

void ey_free_rhs_item_action(ey_rhs_item_action_t *action)
{
	if(!action)
		return;
	if(action->raw_code)
		lexer_free(action->raw_code);
	parser_free(action);
}

ey_rhs_item_t *ey_alloc_rhs_item(ey_location_t *location, 
	char *event_name,
	char *cluster_condition,
	ey_rhs_item_condition_t *condition, 
	ey_rhs_item_action_t *action)
{
	assert(location != NULL);
	assert(event_name != NULL && event_name[0] != 0);
	ey_rhs_item_t *ret = (ey_rhs_item_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->event_name = event_name;
	ret->cluster_condition = cluster_condition;
	ret->condition = condition;
	ret->action = action;
	return ret;
}

void ey_free_rhs_item(ey_rhs_item_t *item)
{
	if(!item)
		return;
	if(item->event_name)
		lexer_free(item->event_name);
	if(item->cluster_condition)
		lexer_free(item->cluster_condition);
	if(item->condition)
		ey_free_rhs_item_condition(item->condition);
	if(item->action)
		ey_free_rhs_item_action(item->action);
	parser_free(item);
}

ey_rhs_signature_t *ey_alloc_rhs_signature(ey_location_t *location, 
	ey_rhs_item_list_t *rhs_list)
{
	assert(location != NULL);
	ey_rhs_signature_t *ret = (ey_rhs_signature_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	TAILQ_INIT(&ret->rhs_item_list);
	if(rhs_list)
		TAILQ_CONCAT(&ret->rhs_item_list, rhs_list, link);
	return ret;
}

void ey_free_rhs_signature(ey_rhs_signature_t *rhs_signature)
{
	if(!rhs_signature)
		return;
	ey_rhs_item_t *item=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(item, &rhs_signature->rhs_item_list, link, tmp)
	{
		ey_free_rhs_item(item);
	}
}

ey_signature_t *ey_alloc_signature(unsigned long id,
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

void ey_free_signature(ey_signature_t *signature)
{
	if(!signature)
		return;
	ey_rhs_signature_t *item = NULL, *tmp = NULL;
	TAILQ_FOREACH_SAFE(item, &signature->rhs_signature_list, link, tmp)
	{
		ey_free_rhs_signature(item);
	}
}

ey_code_t *ey_alloc_code(ey_location_t *location, char *code)
{
	assert(location != NULL);
	ey_code_t *ret = (ey_code_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->raw_code = code;
	return ret;
}

void ey_free_code(ey_code_t *code)
{
	if(!code)
		return;
	if(code->raw_code)
		lexer_free(code->raw_code);
	parser_free(code);
}

ey_signature_file_t *ey_alloc_signature_file(char *output_file,
	ey_code_list_t *prologue_list, 
	ey_signature_list_t *signature_list,
	ey_code_t *epilogue)
{
	ey_signature_file_t *ret = (ey_signature_file_t*)parser_malloc(sizeof(*ret));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->output_file = output_file;
	TAILQ_INIT(&ret->prologue_list);
	if(prologue_list)
		TAILQ_CONCAT(&ret->prologue_list, prologue_list, link);
	TAILQ_INIT(&ret->signature_list);
	if(signature_list)
		TAILQ_CONCAT(&ret->signature_list, signature_list, link);
	ret->epilogue = epilogue;
	return ret;
}

void ey_free_signature_file(ey_signature_file_t *file)
{
	if(!file)
		return;
	
	if(file->output_file)
		parser_free(file->output_file);
	
	ey_code_t *code=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(code, &file->prologue_list, link, tmp)
	{
		ey_free_code(code);
	}
	ey_free_code(file->epilogue);
	
	ey_signature_t *sig=NULL, *sig2=NULL;
	TAILQ_FOREACH_SAFE(sig, &file->signature_list, link, sig2)
	{
		ey_free_signature(sig);
	}
	parser_free(file);
}
