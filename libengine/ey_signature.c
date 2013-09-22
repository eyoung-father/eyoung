#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ey_signature.h"
#include "engine_mem.h"
#include "ey_info.h"
#include "ey_event.h"
#include "ey_engine.h"


ey_rhs_item_condition_t *ey_alloc_rhs_item_condition(ey_engine_t *eng, ey_location_t *location, 
	char *code, char *func_name, void *addr)
{
	assert(location != NULL);

	ey_rhs_item_condition_t *ret = (ey_rhs_item_condition_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->raw_code = code;
	ret->func_name = func_name;
	ret->addr = addr;
	return ret;
}

void ey_free_rhs_item_condition(ey_engine_t *eng, ey_rhs_item_condition_t *condition)
{
	if(!condition)
		return;
	if(condition->raw_code)
		engine_fzfree(ey_parser_fslab(eng), condition->raw_code);
	engine_fzfree(ey_parser_fslab(eng), condition);
}

ey_rhs_item_action_t *ey_alloc_rhs_item_action(ey_engine_t *eng, ey_location_t *location, 
	char *code, char *func_name, void *addr)
{
	assert(location != NULL);

	ey_rhs_item_action_t *ret = (ey_rhs_item_action_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->raw_code = code;
	ret->func_name = func_name;
	ret->addr = addr;
	return ret;
}

void ey_free_rhs_item_action(ey_engine_t *eng, ey_rhs_item_action_t *action)
{
	if(!action)
		return;
	if(action->raw_code)
		engine_fzfree(ey_parser_fslab(eng), action->raw_code);
	engine_fzfree(ey_parser_fslab(eng), action);
}

ey_rhs_item_t *ey_alloc_rhs_item(ey_engine_t *eng, ey_location_t *location, 
	char *event_name,
	ey_acsm_pattern_t *cluster_condition,
	ey_rhs_item_condition_t *condition, 
	ey_rhs_item_action_t *action)
{
	assert(location != NULL);
	assert(event_name != NULL && event_name[0] != 0);
	ey_rhs_item_t *ret = (ey_rhs_item_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->rhs_id = ++ey_rhs_id(eng);
	ret->location = *location;
	ret->event_name = event_name;
	if(cluster_condition)
		cluster_condition->id = (void*)ret->rhs_id;
	ret->cluster_condition = cluster_condition;
	ret->condition = condition;
	ret->action = action;
	return ret;
}

void ey_free_rhs_item(ey_engine_t *eng, ey_rhs_item_t *item)
{
	if(!item)
		return;
	if(item->event_name)
		engine_fzfree(ey_parser_fslab(eng), item->event_name);
	if(item->cluster_condition)
	{
		if(item->cluster_condition->pattern)
			engine_fzfree(ey_parser_fslab(eng), item->cluster_condition->pattern);
		engine_fzfree(ey_parser_fslab(eng), item->cluster_condition);
	}
	if(item->condition)
		ey_free_rhs_item_condition(eng, item->condition);
	if(item->action)
		ey_free_rhs_item_action(eng, item->action);
	engine_fzfree(ey_parser_fslab(eng), item);
}

ey_rhs_signature_t *ey_alloc_rhs_signature(ey_engine_t *eng, ey_location_t *location, 
	ey_rhs_item_list_t *rhs_list)
{
	assert(location != NULL);
	ey_rhs_signature_t *ret = (ey_rhs_signature_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	TAILQ_INIT(&ret->rhs_item_list);
	if(rhs_list)
		TAILQ_CONCAT(&ret->rhs_item_list, rhs_list, link);
	return ret;
}

void ey_free_rhs_signature(ey_engine_t *eng, ey_rhs_signature_t *rhs_signature)
{
	if(!rhs_signature)
		return;
	ey_rhs_item_t *item=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(item, &rhs_signature->rhs_item_list, link, tmp)
	{
		ey_free_rhs_item(eng, item);
	}
}

ey_signature_t *ey_alloc_signature(ey_engine_t *eng, unsigned int id,
	ey_location_t *location, ey_rhs_signature_list_t *signature_list)
{
	assert(location != NULL);
	assert(signature_list != NULL);
	ey_signature_t *ret = (ey_signature_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->signature_id = id;
	ret->location = *location;
	TAILQ_INIT(&ret->rhs_signature_list);
	TAILQ_CONCAT(&ret->rhs_signature_list, signature_list, link);
	return ret;
}

void ey_free_signature(ey_engine_t *eng, ey_signature_t *signature)
{
	if(!signature)
		return;
	ey_rhs_signature_t *item = NULL, *tmp = NULL;
	TAILQ_FOREACH_SAFE(item, &signature->rhs_signature_list, link, tmp)
	{
		ey_free_rhs_signature(eng, item);
	}
}

ey_code_t *ey_alloc_code(ey_engine_t *eng, ey_location_t *location, void *code, int type)
{
	assert(location != NULL);
	ey_code_t *ret = (ey_code_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->type = type;
	switch(type)
	{
		case EY_CODE_NORMAL:
			ret->raw_code = (char*)code;
			break;
		case EY_CODE_IMPORT:
			ret->filename = (char*)code;
			break;
		case EY_CODE_EVENT:
		default:
			ret->event = (ey_event_t*)code;
			break;
	}
	return ret;
}

void ey_free_code(ey_engine_t *eng, ey_code_t *code)
{
	if(!code)
		return;
	switch(code->type)
	{
		case EY_CODE_NORMAL:
			if(code->raw_code) engine_fzfree(ey_parser_fslab(eng), code->raw_code);
			break;
		case EY_CODE_IMPORT:
			if(code->filename) engine_fzfree(ey_parser_fslab(eng), code->filename);
			break;
		case EY_CODE_EVENT:
		default:
			if(code->event) ey_free_event(eng, code->event);
			break;
	}
	engine_fzfree(ey_parser_fslab(eng), code);
}

ey_signature_file_t *ey_alloc_signature_file(ey_engine_t *eng, char *output_file,
	ey_code_list_t *prologue_list, 
	ey_signature_list_t *signature_list,
	ey_code_t *epilogue)
{
	ey_signature_file_t *ret = (ey_signature_file_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
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

void ey_free_signature_file(ey_engine_t *eng, ey_signature_file_t *file)
{
	if(!file)
		return;
	
	if(file->output_file)
		engine_fzfree(ey_parser_fslab(eng), file->output_file);
	
	ey_code_t *code=NULL, *tmp=NULL;
	TAILQ_FOREACH_SAFE(code, &file->prologue_list, link, tmp)
	{
		ey_free_code(eng, code);
	}
	ey_free_code(eng, file->epilogue);
	
	ey_signature_t *sig=NULL, *sig2=NULL;
	TAILQ_FOREACH_SAFE(sig, &file->signature_list, link, sig2)
	{
		ey_free_signature(eng, sig);
	}
	engine_fzfree(ey_parser_fslab(eng), file);
}

static unsigned int hash_signature(void *signature)
{
	return (unsigned int)signature;
}

static int compare_signature(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return *(unsigned int*)k != ((ey_signature_t*)v)->signature_id;
}


int ey_signature_init(struct ey_engine *eng)
{
	char name[128];
	if(!ey_signature_hash(eng))
	{
		snprintf(name, sizeof(name), "%s signature hash\n", eng->name);
		name[63] = '\0';
		ey_signature_hash(eng) = ey_hash_create(name, 10, 8192, hash_signature, compare_signature, NULL, NULL);
		if(!ey_signature_hash(eng))
		{
			engine_init_error("create signature hash failed\n");
			return -1;
		}
	}

	return 0;
}

void ey_signature_finit(struct ey_engine *eng)
{
	if(!eng)
		return;

	if(ey_signature_hash(eng))
	{
		ey_hash_destroy(ey_signature_hash(eng));
		ey_signature_hash(eng) = NULL;
	}
}

ey_signature_t *ey_find_signature(ey_engine_t *eng, unsigned int id)
{
	if(!eng|| !ey_signature_hash(eng))
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return NULL;
	}

	return ey_hash_find(ey_signature_hash(eng), (void*)&id);
}

int ey_insert_signature(ey_engine_t *eng, ey_signature_t *signature)
{
	if(!eng || !ey_signature_hash(eng) || !signature)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	ey_signature_t *find = ey_find_signature(eng, signature->signature_id);
	if(find)
	{
		engine_parser_error("signature %lu is already inserted\n",signature->signature_id);
		return -1;
	}

	if(ey_hash_insert(ey_signature_hash(eng), (void*)&signature->signature_id, signature))
	{
		engine_parser_error("signature %lu inserted failed\n", signature->signature_id);
		return -1;
	}
	engine_parser_debug("signature %lu inserted successfully\n", signature->signature_id);
	return 0;
}
