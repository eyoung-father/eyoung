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
	ey_assert(location != NULL);

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
	ey_assert(location != NULL);

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
	ey_assert(location != NULL);
	ey_assert(event_name != NULL && event_name[0] != 0);
	ey_rhs_item_t *ret = (ey_rhs_item_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->rhs_id = ++ey_rhs_id(eng);
	ret->location = *location;
	ret->event_name = event_name;
	if(cluster_condition)
		cluster_condition->id = (void*)(unsigned long)ret->rhs_id;
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
	ey_assert(location != NULL);
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

ey_signature_t *ey_alloc_signature(ey_engine_t *eng, unsigned long id,
	ey_location_t *location, ey_rhs_signature_list_t *signature_list)
{
	ey_assert(location != NULL);
	ey_assert(signature_list != NULL);
	ey_signature_t *ret = (ey_signature_t*)engine_fzalloc(sizeof(*ret), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->signature_id = id;
	ret->location = *location;
	TAILQ_INIT(&ret->rhs_signature_list);
	TAILQ_CONCAT(&ret->rhs_signature_list, signature_list, link);

	unsigned long rhs_pos = 0;
	ey_rhs_signature_t *rhs = NULL;
	TAILQ_FOREACH(rhs, &ret->rhs_signature_list, link)
	{
		rhs->signature_id = id;
		rhs->rhs_signature_position = rhs_pos++;

		ey_rhs_item_t *item = NULL;
		unsigned long item_pos = 0;
		TAILQ_FOREACH(item, &rhs->rhs_item_list, link)
		{
			item->signature_id = id;
			item->rhs_signature_position = rhs->rhs_signature_position;
			item->rhs_item_position = item_pos++;
		}
	}
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

ey_code_t *ey_alloc_code(ey_engine_t *eng, ey_location_t *location, void *code, void *addr, void *event, int type)
{
	ey_assert(location != NULL);
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
		case EY_CODE_FILE_INIT:
		case EY_CODE_FILE_FINIT:
		case EY_CODE_WORK_INIT:
		case EY_CODE_WORK_FINIT:
		case EY_CODE_EVENT_INIT:
		case EY_CODE_EVENT_FINIT:
		case EY_CODE_EVENT_PREPROCESSOR:
			ret->function = (char*)code;
			ret->handle = addr;
			ret->event_name = (char*)event;
			break;
		case EY_CODE_EVENT:
		default:
			ret->event = ((ey_event_t*)code)->event_id;
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
			if(code->raw_code) 
				engine_fzfree(ey_parser_fslab(eng), code->raw_code);
			break;
		case EY_CODE_IMPORT:
			if(code->filename) 
				engine_fzfree(ey_parser_fslab(eng), code->filename);
			break;
		case EY_CODE_FILE_INIT:
		case EY_CODE_FILE_FINIT:
		case EY_CODE_WORK_INIT:
		case EY_CODE_WORK_FINIT:
		case EY_CODE_EVENT_INIT:
		case EY_CODE_EVENT_FINIT:
		case EY_CODE_EVENT_PREPROCESSOR:
			if(code->function) 
				engine_fzfree(ey_parser_fslab(eng), code->function);
			if(code->event_name)
				engine_fzfree(ey_parser_fslab(eng), code->event_name);
			break;
		case EY_CODE_EVENT:
		default:
			ey_free_event(eng, ey_event_array(eng) + code->event);
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

static unsigned long hash_signature(void *signature)
{
	return (unsigned long)signature;
}

static int compare_signature(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return *(unsigned long*)k != ((ey_signature_t*)v)->signature_id;
}

static unsigned long hash_rhs_item(void *rhs_item)
{
	return (unsigned long)rhs_item;
}

static int compare_rhs_item(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return *(unsigned long*)k != ((ey_rhs_item_t*)v)->rhs_id;
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

	if(!ey_rhs_item_hash(eng))
	{
		snprintf(name, sizeof(name), "%s signature rhs item hash\n", eng->name);
		name[63] = '\0';
		ey_rhs_item_hash(eng) = ey_hash_create(name, 10, 8192, hash_rhs_item, compare_rhs_item, NULL, NULL);
		if(!ey_rhs_item_hash(eng))
		{
			engine_init_error("create signature rhs item hash failed\n");
			return -1;
		}
	}

	TAILQ_INIT(&ey_signature_list(eng));
	TAILQ_INIT(&ey_file_init_list(eng));
	TAILQ_INIT(&ey_file_finit_list(eng));
	return 0;
}

void ey_signature_finit(struct ey_engine *eng)
{
	if(!eng)
		return;
	
	ey_code_t *func = NULL, *tmp = NULL;
	/*free ey_file_finit_list*/
	TAILQ_FOREACH_SAFE(func, &ey_file_finit_list(eng), link, tmp)
	{
		if(func->handle)
			((init_handler)(func->handle))(eng);	/*call finit in signature file*/
		ey_free_code(eng, func);
	}
	
	/*free ey_file_init_list*/
	TAILQ_FOREACH_SAFE(func, &ey_file_init_list(eng), link, tmp)
	{
		ey_free_code(eng, func);
	}
	
	if(ey_signature_hash(eng))
	{
		ey_hash_destroy(ey_signature_hash(eng));
		ey_signature_hash(eng) = NULL;
	}

	if(ey_rhs_item_hash(eng))
	{
		ey_hash_destroy(ey_rhs_item_hash(eng));
		ey_rhs_item_hash(eng) = NULL;
	}
}

ey_signature_t *ey_find_signature(ey_engine_t *eng, unsigned long id)
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

	ey_rhs_signature_t *rhs = NULL;
	TAILQ_FOREACH(rhs, &signature->rhs_signature_list, link)
	{
		ey_rhs_item_t *item = NULL;
		TAILQ_FOREACH(item, &rhs->rhs_item_list, link)
		{
			ey_event_t *event = ey_find_event(eng, item->event_name);
			ey_assert(event!=NULL);
			item->clustered = 0;
			if(item->cluster_condition)
				item->clustered = 1;
			TAILQ_INSERT_TAIL(&event->item_list, item, event_link);
		}
	}
	engine_parser_debug("signature %lu inserted successfully\n", signature->signature_id);
	return 0;
}

int ey_signature_add_init(ey_engine_t *eng, const char *function, 
	file_init_handle address, ey_location_t *location)
{
	if(!eng || !function || !location)
	{
		engine_init_error("%s bad parameter\b", __FUNCTION__);
		return -1;
	}

	ey_code_t *find = NULL;
	TAILQ_FOREACH(find, &ey_file_init_list(eng), link)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("file init function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
	}
	
	char *name = (char*)engine_fzalloc(strlen((char*)function)+1, ey_parser_fslab(eng));
	if(!name)
	{
		engine_init_error("malloc init function name failed\n");
		return -1;
	}
	strcpy(name, function);

	find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_FILE_INIT);
	if(!find)
	{
		engine_init_error("copy file init function %s failed\n", function);
		return -1;
	}
	TAILQ_INSERT_TAIL(&ey_file_init_list(eng), find, link);
	return 0;
}

int ey_signature_add_finit(ey_engine_t *eng, const char *function, 
	file_finit_handle address, ey_location_t *location)
{
	if(!eng || !function || !location)
	{
		engine_init_error("%s bad parameter\b", __FUNCTION__);
		return -1;
	}

	ey_code_t *find = NULL;
	TAILQ_FOREACH(find, &ey_file_finit_list(eng), link)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("file finit function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
	}

	char *name = (char*)engine_fzalloc(strlen((char*)function)+1, ey_parser_fslab(eng));
	if(!name)
	{
		engine_init_error("malloc init function name failed\n");
		return -1;
	}
	strcpy(name, function);

	find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_FILE_FINIT);
	if(!find)
	{
		engine_init_error("copy file finit function %s failed\n", function);
		return -1;
	}
	TAILQ_INSERT_TAIL(&ey_file_finit_list(eng), find, link);
	return 0;
}
