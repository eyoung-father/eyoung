#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ey_event.h"
#include "ey_memory.h"
#include "engine_mem.h"
#include "ey_engine.h"
#include "ey_info.h"

int ey_event_init(ey_engine_t *eng)
{
	if(!ey_event_array(eng))
	{
		ey_event_array(eng) = (ey_event_t*)engine_calloc(EVENT_ARRAY_STEP, sizeof(ey_event_t));
		if(!ey_event_array(eng))
		{
			engine_init_error("alloc event array failed\n");
			return -1;
		}
		ey_event_size(eng) = EVENT_ARRAY_STEP;

		int index = 0;
		ey_event_t *ev = ey_event_array(eng);
		for(; index<EVENT_ARRAY_STEP; index++, ev++)
			TAILQ_INIT(&ev->item_list);
	}
	ey_event_count(eng) = 0;

	return 0;
}

void ey_event_finit(ey_engine_t *eng)
{
	if(!eng)
		return;

	if(ey_event_array(eng))
	{
		int index = 0;
		ey_event_t *event = NULL;
		for(index=0, event=ey_event_array(eng); index<ey_event_count(eng); index++, event++)
			ey_free_event(eng, event);
		engine_free(ey_event_array(eng));
		ey_event_array(eng) = NULL;
		ey_event_size(eng) = 0;
		ey_event_count(eng) = 0;
	}
}

ey_event_t *ey_find_event(ey_engine_t *eng, const char *name)
{
	if(!eng || !name)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return NULL;
	}
	
	int index;
	for(index=0; index<ey_event_count(eng); index++)
	{
		if(!strcmp(name, ey_event_array(eng)[index].name))
			return ey_event_array(eng) + index;
	}

	return NULL;
}

ey_event_t *ey_alloc_event(ey_engine_t *eng, ey_location_t *location, char *name, char *define)
{
	ey_event_t *find = ey_find_event(eng, name);
	if(find)
	{
		if(strcmp(find->define, define))
		{
			engine_parser_error("event %s is already defined in %s:%d\n", find->location.filename, find->location.first_line);
			return NULL;
		}
		return find;
	}
	
	ey_event_t *ret = NULL;
	if(ey_event_count(eng) > ey_event_size(eng))
	{
		engine_parser_error("fatal error!\n");
		*(int*)0 = 0;
	}
	else if(ey_event_count(eng) == ey_event_size(eng))
	{
		int new_size = ey_event_size(eng) + EVENT_ARRAY_STEP;
		ey_event_t *new_array = (ey_event_t*)engine_calloc(new_size, sizeof(ey_event_t));
		if(!new_array)
		{
			engine_parser_error("alloc new event array failed\n");
			return NULL;
		}
		memcpy(new_array, ey_event_array(eng), sizeof(ey_event_t)*ey_event_count(eng));

		int index = 0;
		ey_event_t *ev = new_array;
		for(; index<new_size; index++, ev++)
		{
			TAILQ_INIT(&ev->item_list);
			if(index<ey_event_size(eng))
			{
				ey_event_t *ev_old = ey_event_array(eng) + index;
				TAILQ_CONCAT(&ev->item_list, &ev_old->item_list, event_link);
			}
		}

		engine_free(ey_event_array(eng));
		ey_event_array(eng) = new_array;
		ey_event_size(eng) = new_size;
	}
	ret = ey_event_array(eng) + ey_event_count(eng);
	memset(ret, 0, sizeof(*ret));
	ret->event_id = ey_event_count(eng);
	ret->location = *location;
	ret->name = name;
	ret->define = define;
	TAILQ_INIT(&ret->item_list);
	ey_event_count(eng)++;
	return ret;
}

void ey_free_event(ey_engine_t *eng, ey_event_t *event)
{
	if(!eng || !event)
		return;
	
	if(event->name)
		engine_fzfree(ey_parser_fslab(eng), event->name);
	
	if(event->define)
		engine_fzfree(ey_parser_fslab(eng), event->define);

}

int ey_event_set_runtime_init(ey_engine_t *eng, const char *event_name, int user_define,
	const char *function, event_init_handle address, ey_location_t *location)
{
	if(!eng || !event_name || !function || !location)
	{
		engine_init_error("%s bad parameters\n", __FUNCTION__);
		return -1;
	}

	ey_event_t *event = ey_find_event(eng, event_name);
	if(!event)
	{
		engine_init_error("event %s is not defined\n", event_name);
		return -1;
	}

	ey_code_t *find = NULL;
	if(user_define)
		find = event->event_init_userdefined;
	else
		find = event->event_init_predefined;
	if(find)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("event init function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
		else
		{
			engine_init_error("event init function is already set in %s:%d\n", find->location.filename, find->location.first_line);
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

		find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_EVENT_INIT);
		if(!find)
		{
			engine_init_error("copy event init function %s failed\n", function);
			engine_fzfree(ey_parser_fslab(eng), name);
			return -1;
		}
		if(user_define)
			event->event_init_userdefined = find;
		else
			event->event_init_predefined = find;
		return 0;
	}
}

int ey_event_set_runtime_preprocessor(ey_engine_t *eng, const char *event_name, int user_define,
	const char *function, event_preprocess_handle address, ey_location_t *location)
{
	if(!eng || !event_name || !function || !location)
	{
		engine_init_error("%s bad parameters\n", __FUNCTION__);
		return -1;
	}

	ey_event_t *event = ey_find_event(eng, event_name);
	if(!event)
	{
		engine_init_error("event %s is not defined\n", event_name);
		return -1;
	}

	ey_code_t *find = NULL;
	if(user_define)
		find = event->event_preprocessor_userdefined;
	else
		find = event->event_preprocessor_predefined;
	if(find)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("event preprocessor function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
		else
		{
			engine_init_error("event preprocessor function is already set in %s:%d\n", find->location.filename, find->location.first_line);
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

		find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_EVENT_PREPROCESSOR);
		if(!find)
		{
			engine_init_error("copy event preprocessor function %s failed\n", function);
			engine_fzfree(ey_parser_fslab(eng), name);
			return -1;
		}
		if(user_define)
			event->event_preprocessor_userdefined = find;
		else
			event->event_preprocessor_predefined = find;
		return 0;
	}
}

int ey_event_set_runtime_finit(ey_engine_t *eng, const char *event_name, int user_define,
	const char *function, event_finit_handle address, ey_location_t *location)
{
	if(!eng || !event_name || !function || !location)
	{
		engine_init_error("%s bad parameters\n", __FUNCTION__);
		return -1;
	}

	ey_event_t *event = ey_find_event(eng, event_name);
	if(!event)
	{
		engine_init_error("event %s is not defined\n", event_name);
		return -1;
	}

	ey_code_t *find = NULL;
	if(user_define)
		find = event->event_finit_userdefined;
	else
		find = event->event_finit_predefined;
	if(find)
	{
		if(!strcmp(function, find->function))
		{
			engine_init_debug("event finit function is already called %s:%d\n", find->location.filename, find->location.first_line);
			return 0;
		}
		else
		{
			engine_init_error("event finit function is already set in %s:%d\n", find->location.filename, find->location.first_line);
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

		find = ey_alloc_code(eng, location, name, address, NULL, EY_CODE_EVENT_FINIT);
		if(!find)
		{
			engine_init_error("copy event finit function %s failed\n", function);
			engine_fzfree(ey_parser_fslab(eng), name);
			return -1;
		}
		if(user_define)
			event->event_finit_userdefined = find;
		else
			event->event_finit_predefined = find;
		return 0;
	}
}
