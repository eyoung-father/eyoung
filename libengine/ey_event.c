#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ey_event.h"
#include "ey_memory.h"
#include "engine_mem.h"
#include "ey_engine.h"
#include "ey_info.h"

static unsigned int hash_event(void *event)
{
	return (unsigned int)event;
}

static int compare_event(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return strcmp((char*)k, ((ey_event_t*)v)->name);
}

int ey_event_init(ey_engine_t *eng)
{
	char name[128];
	if(!ey_event_hash(eng))
	{
		snprintf(name, sizeof(name), "%s event hash\n", eng->name);
		name[63] = '\0';
		ey_event_hash(eng) = ey_hash_create(name, 10, 8192, hash_event, compare_event, NULL, NULL);
		if(!ey_event_hash(eng))
		{
			engine_init_error("create event hash failed\n");
			return -1;
		}
	}

	return 0;
}

void ey_event_finit(ey_engine_t *eng)
{
	if(!eng)
		return;

	if(ey_event_hash(eng))
	{
		ey_hash_destroy(ey_event_hash(eng));
		ey_event_hash(eng) = NULL;
	}
}

ey_event_t *ey_define_event(ey_engine_t *eng, ey_event_t *event)
{
	if(!eng || !ey_event_hash(eng) || !event)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return NULL;
	}
	
	ey_event_t *ret = ey_find_event(eng, event->name);
	if(ret)
	{
		if(strcmp(event->define, ret->define))
		{
			engine_parser_error("event %s is already defined in %s:%d\n", event->name, ret->location.filename, ret->location.first_line);
			return NULL;
		}
		return event;
	}

	if(ey_hash_insert(ey_event_hash(eng), (void*)event->name, event))
	{
		engine_parser_error("insert hash for event %s failed\n", event->name);
		return NULL;
	}
	engine_parser_debug("define event %s successfully\n", event->name);
	return event;
}

ey_event_t *ey_find_event(ey_engine_t *eng, char *name)
{
	if(!eng || !name)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return NULL;
	}
	
	return ey_hash_find(ey_event_hash(eng), (void*)name);
}

ey_event_t *ey_alloc_event(ey_engine_t *eng, ey_location_t *location, char *name, char *define)
{
	ey_event_t *ret = (ey_event_t*)engine_fzalloc(sizeof(ey_event_t), ey_parser_fslab(eng));
	if(!ret)
		return NULL;
	memset(ret, 0, sizeof(*ret));
	ret->location = *location;
	ret->name = name;
	ret->define = define;
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
	
	engine_fzfree(ey_parser_fslab(eng), event);
}
