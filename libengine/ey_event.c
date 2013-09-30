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
		engine_free(ey_event_array(eng));
		ey_event_array(eng) = NULL;
		ey_event_size(eng) = 0;
		ey_event_count(eng) = 0;
	}
}

ey_event_t *ey_find_event(ey_engine_t *eng, char *name)
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
	
	ey_acsm_t ac = ey_acsm_create();
	if(!ac)
	{
		engine_parser_error("create acsm structure failed\n");
		return NULL;
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
			ey_acsm_destroy(ac);
			return NULL;
		}
		memcpy(new_array, ey_event_array(eng), sizeof(ey_event_t)*ey_event_count(eng));
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
	ret->cluster_pattern = ac;
	TAILQ_INIT(&ret->cluster_item_list);
	TAILQ_INIT(&ret->uncluster_item_list);
	TAILQ_INIT(&ret->event_init_list);
	TAILQ_INIT(&ret->event_finit_list);
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

	if(event->cluster_pattern)
		ey_acsm_destroy(event->cluster_pattern);
}
