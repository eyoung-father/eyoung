#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ey_engine.h"
#include "ey_runtime.h"

int ey_runtime_init(ey_engine_t *eng)
{
	if(ey_bitmap_init(eng))
	{
		engine_init_error("init bitmap failed\n");
		return -1;
	}

	if(ey_work_init(eng))
	{
		engine_init_error("init work failed\n");
		return -1;
	}
	return 0;
}

void ey_runtime_finit(ey_engine_t *eng)
{
	ey_work_finit(eng);
	ey_bitmap_finit(eng);
	return;
}

engine_work_t* ey_runtime_create(ey_engine_t *eng)
{
	return NULL;
}

void ey_runtime_destroy(engine_work_t *work)
{
	return;
}

engine_work_event_t *ey_runtime_create_event(engine_work_t *work, unsigned long event_id, engine_action_t *action)
{
	return NULL;
}

int ey_runtime_detect_event(engine_work_event_t *event)
{
	return 0;
}

void ey_runtime_destroy_event(engine_work_event_t *event)
{
	return;
}
