#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ey_engine.h"
#include "ey_work.h"

int ey_work_set_runtime_init(ey_engine_t *eng, int user_define, 
	const char *function, work_init_handle address, ey_location_t *location)
{
	if(!eng || !function || !location)
	{
		engine_init_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	ey_code_t *find = NULL;
	if(user_define)
		find = ey_work_init_userdefined(eng);
	else
		find = ey_work_init_predefined(eng);
	if(find)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("work init function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
		else
		{
			engine_init_error("user-defined work init function is already set in %s:%d\n", 
				find->location.filename, find->location.first_line);
			return -1;
		}
	}
	else
	{
		char *name = engine_fzalloc(strlen(function)+1, ey_parser_fslab(eng));
		if(!name)
		{
			engine_init_error("alloc function name failed\n");
			return -1;
		}
		strcpy(name, function);

		find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_WORK_INIT);
		if(!find)
		{
			engine_init_error("copy work init function %s failed\n", function);
			engine_fzfree(ey_parser_fslab(eng), name);
			return -1;
		}
		if(user_define)
			ey_work_init_userdefined(eng) = find;
		else
			ey_work_init_predefined(eng) = find;
		return 0;
	}
}

int ey_work_set_runtime_finit(ey_engine_t *eng, int user_define, 
	const char *function, work_finit_handle address, ey_location_t *location)
{
	if(!eng || !function || !location)
	{
		engine_init_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	ey_code_t *find = NULL;
	if(user_define)
		find = ey_work_finit_userdefined(eng);
	else
		find = ey_work_finit_predefined(eng);
	if(find)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("work finit function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
		else
		{
			engine_init_error("user-defined work finit function is already set in %s:%d\n", 
				find->location.filename, find->location.first_line);
			return -1;
		}
	}
	else
	{
		char *name = engine_fzalloc(strlen(function)+1, ey_parser_fslab(eng));
		if(!name)
		{
			engine_init_error("alloc function name failed\n");
			return -1;
		}
		strcpy(name, function);

		find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_WORK_FINIT);
		if(!find)
		{
			engine_init_error("copy work finit function %s failed\n", function);
			engine_fzfree(ey_parser_fslab(eng), name);
			return -1;
		}
		if(user_define)
			ey_work_finit_userdefined(eng) = find;
		else
			ey_work_finit_predefined(eng) = find;
		return 0;
	}
}

static unsigned long hash_engine_work(void *work)
{
	return (unsigned long)work;
}

static int compare_engine_work(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return *(unsigned long*)k != ((engine_work_t*)v)->work_id;
}

int ey_work_init(ey_engine_t *eng)
{
	ey_work_slab(eng) = engine_zinit("private work slab", sizeof(ey_work_t));
	if(!ey_work_slab(eng))
	{
		engine_init_error("init private work slab failed\n");
		return -1;
	}
	
	ey_engine_work_slab(eng) = engine_zinit("engine work slab", sizeof(engine_work_t));
	if(!ey_engine_work_slab(eng))
	{
		engine_init_error("init engine work slab failed\n");
		return -1;
	}

	ey_engine_work_event_slab(eng) = engine_zinit("engine work event slab", sizeof(engine_work_event_t));
	if(!ey_engine_work_event_slab(eng))
	{
		engine_init_error("init engine work event slab failed\n");
		return -1;
	}
	
	char name[64];
	if(!ey_engine_work_hash(eng))
	{
		snprintf(name, sizeof(name), "%s engine work hash\n", eng->name);
		name[63] = '\0';
		ey_engine_work_hash(eng) = ey_hash_create(name, 20, 2^20, hash_engine_work, compare_engine_work, NULL, NULL);
		if(!ey_engine_work_hash(eng))
		{
			engine_init_error("create engine work hash failed\n");
			return -1;
		}
	}
	return 0;
}

void ey_work_finit(struct ey_engine *eng)
{
	if(!eng)
		return;
	
	if(ey_engine_work_hash(eng))
	{
		/*TODO: call destroy function for each item*/
		ey_hash_destroy(ey_engine_work_hash(eng));
		ey_engine_work_hash(eng) = NULL;
	}

	if(ey_engine_work_event_slab(eng))
	{
		engine_zfinit(ey_engine_work_event_slab(eng));
		ey_engine_work_event_slab(eng) = NULL;
	}

	if(ey_engine_work_slab(eng))
	{
		engine_zfinit(ey_engine_work_slab(eng));
		ey_engine_work_slab(eng) = NULL;
	}

	if(ey_work_slab(eng))
	{
		engine_zfinit(ey_work_slab(eng));
		ey_work_slab(eng) = NULL;
	}
}
