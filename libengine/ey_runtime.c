#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ey_engine.h"
#include "ey_runtime.h"

typedef struct ey_runtime_item
{
	int *results;
	ey_spinlock_t lock;
}ey_runtime_item_t;
static int *result_buffer;
static ey_runtime_item_t runtime_item[MAX_RUNTIME_ITEM];

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

	result_buffer = (int*)engine_malloc(sizeof(int) * ey_rhs_id(eng) * MAX_RUNTIME_ITEM);
	if(!result_buffer)
	{
		engine_init_error("malloc result buffer failed\n");
		return -1;
	}
	memset(result_buffer, 0, sizeof(int) * ey_rhs_id(eng) * MAX_RUNTIME_ITEM);

	int index;
	for(index=0; index<MAX_RUNTIME_ITEM; index++)
	{
		runtime_item[index].results = result_buffer + ey_rhs_id(eng) * index;
		ey_spinlock_init(&runtime_item[index].lock);
	}
	return 0;
}

void ey_runtime_finit(ey_engine_t *eng)
{
	if(result_buffer)
		engine_free(result_buffer);
	memset(runtime_item, 0, sizeof(runtime_item));

	ey_work_finit(eng);
	ey_bitmap_finit(eng);
	return;
}

engine_work_t* ey_runtime_create(ey_engine_t *eng)
{
	assert(eng!=NULL);
	ey_bitmap_t *bitmap = NULL;
	ey_work_t *ey_work = NULL;
	engine_work_t *engine_work = NULL;

	/*alloc bitmap*/
	bitmap = ey_bitmap_create(eng, ey_rhs_id(eng));
	if(!bitmap)
	{
		engine_runtime_error("alloc bitmap failed\n");
		goto common_failed;
	}

	/*alloc runtime ey_work*/
	ey_work = (ey_work_t*)engine_zalloc(ey_work_slab(eng));
	if(!ey_work)
	{
		engine_runtime_error("alloc ey_work failed\n");
		goto common_failed;
	}
	memset(ey_work, 0, sizeof(*ey_work));

	/*init ey_work*/
	ey_work->local_allocator = engine_fzinit("", 64, NULL);
	if(!ey_work->local_allocator)
	{
		engine_runtime_error("alloc work fslab failed\n");
		goto common_failed;
	}
	ey_work->state_bitmap = bitmap;
	TAILQ_INIT(&ey_work->event_list);
	ey_spinlock_init(&ey_work->work_lock);

	/*alloc engine work*/
	engine_work = (engine_work_t*)engine_zalloc(ey_engine_work_slab(eng));
	if(!engine_work)
	{
		engine_runtime_error("alloc engine work failed\n");
		goto common_failed;
	}
	memset(engine_work, 0, sizeof(*engine_work));

	/*init engine work*/
	engine_work->engine = (void*)eng;
	engine_work->priv_data = (void*)ey_work;

	/*call predefined engine work function*/
	if(ey_work_init_predefined(eng))
	{
		work_init_handle predefined_init = (work_init_handle)(ey_work_init_predefined(eng)->handle);
		assert(predefined_init != NULL);
		if(predefined_init(engine_work))
		{
			engine_runtime_error("call predefined work initializer failed\n");
			goto predefined_failed;
		}
	}

	/*call userdefined engine work function*/
	if(ey_work_init_userdefined(eng))
	{
		work_init_handle userdefined_init = (work_init_handle)(ey_work_init_userdefined(eng)->handle);
		assert(userdefined_init != NULL);
		if(userdefined_init(engine_work))
		{
			engine_runtime_error("call userdefined work initializer failed");
			goto userdefined_failed;
		}
	}

	/*insert into work list*/
	ey_spinlock_lock(&ey_engine_lock(eng));
	TAILQ_INSERT_TAIL(&ey_engine_work_list(eng), engine_work, link);
	ey_spinlock_unlock(&ey_engine_lock(eng));
	return engine_work;

userdefined_failed:
	/*call userdefined engine work finit function*/
	if(ey_work_finit_userdefined(eng))
	{
		work_finit_handle userdefined_finit = (work_finit_handle)(ey_work_finit_userdefined(eng)->handle);
		assert(userdefined_finit != NULL);
		if(userdefined_finit(engine_work))
			engine_runtime_error("call userdefined work finalializer failed");
	}
	
predefined_failed:
	/*call predefined engine work finit function*/
	if(ey_work_finit_predefined(eng))
	{
		work_finit_handle predefined_finit = (work_finit_handle)(ey_work_finit_predefined(eng)->handle);
		assert(predefined_finit != NULL);
		if(predefined_finit(engine_work))
			engine_runtime_error("call predefined work finalializer failed");
	}
	
common_failed:
	if(engine_work)
		engine_zfree(ey_engine_work_slab(eng), engine_work);

	if(ey_work)
	{
		if(ey_work->local_allocator)
			engine_fzfinit(ey_work->local_allocator);
		engine_zfree(ey_work_slab(eng), ey_work);
	}

	if(bitmap)
		ey_bitmap_destroy(eng, bitmap);
	return NULL;
}

void ey_runtime_destroy(engine_work_t *work)
{
	assert(work!=NULL && work->engine!=NULL && work->priv_data);
	ey_engine_t *eng = (ey_engine_t*)(work->engine);

	/*remove from work list*/
	ey_spinlock_lock(&ey_engine_lock(eng));
	TAILQ_REMOVE(&ey_engine_work_list(eng), work, link);
	ey_spinlock_unlock(&ey_engine_lock(eng));

	ey_work_t *priv_work = (ey_work_t*)(work->priv_data);
	assert(priv_work!=NULL);

	/*release work event list*/
	engine_work_event_t *event = NULL, *tmp = NULL;
	ey_spinlock_lock(&priv_work->work_lock);
	TAILQ_FOREACH_SAFE(event, &priv_work->event_list, link, tmp)
	{
		ey_runtime_destroy_event(event);
	}
	ey_spinlock_unlock(&priv_work->work_lock);

	/*call userdefined engine work finit function*/
	if(ey_work_finit_userdefined(eng))
	{
		work_finit_handle userdefined_finit = (work_finit_handle)(ey_work_finit_userdefined(eng)->handle);
		assert(userdefined_finit != NULL);
		if(userdefined_finit(work))
			engine_runtime_error("call userdefined work finalializer failed");
	}

	/*call predefined engine work finit function*/
	if(ey_work_finit_predefined(eng))
	{
		work_finit_handle predefined_finit = (work_finit_handle)(ey_work_finit_predefined(eng)->handle);
		assert(predefined_finit != NULL);
		if(predefined_finit(work))
			engine_runtime_error("call predefined work finalializer failed");
	}
	
	/*common finit*/
	if(priv_work->local_allocator)
		engine_fzfinit(priv_work->local_allocator);
	
	if(priv_work->state_bitmap)
		ey_bitmap_destroy(eng, priv_work->state_bitmap);
	
	engine_zfree(ey_work_slab(eng), priv_work);
	engine_zfree(ey_engine_work_slab(eng), work);
	return;
}

engine_work_event_t *ey_runtime_create_event(engine_work_t *work, unsigned long event_id, engine_action_t *action)
{
	assert(action!=NULL);
	action->action = ENGINE_ACTION_PASS;

	assert(work!=NULL && work->engine!=NULL && work->priv_data!=NULL);
	ey_engine_t *eng = (ey_engine_t*)(work->engine);

	assert(event_id <= ey_event_count(eng));
	ey_event_t *event = ey_event_array(eng) + event_id;

	engine_work_event_t *work_event = (engine_work_event_t*)engine_zalloc(ey_engine_work_event_slab(eng));
	if(!work_event)
	{
		engine_runtime_error("alloc work event failed\n");
		goto common_failed;
	}
	memset(work_event, 0, sizeof(*work_event));
	work_event->work = work;
	work_event->event = (void*)event;
	work_event->action = action;

	/*call predefined init function*/
	if(event->event_init_predefined)
	{
		event_init_handle predefined_init = (event_init_handle)(event->event_init_predefined->handle);
		assert(predefined_init != NULL);
		if(predefined_init(work_event))
		{
			engine_runtime_error("call predefined work event initializer failed\n");
			goto predefined_failed;
		}
	}

	/*call userdefined init function*/
	if(event->event_init_userdefined)
	{
		event_init_handle userdefined_init = (event_init_handle)(event->event_init_userdefined->handle);
		assert(userdefined_init != NULL);
		if(userdefined_init(work_event))
		{
			engine_runtime_error("call userdefined work event initializer failed\n");
			goto userdefined_failed;
		}
	}
	return work_event;

userdefined_failed:
	if(event->event_finit_userdefined)
	{
		event_finit_handle userdefined_finit = (event_finit_handle)(event->event_finit_userdefined->handle);
		assert(userdefined_finit != NULL);
		if(userdefined_finit(work_event))
			engine_runtime_error("call userdefined work event finitializer failed\n");
	}

predefined_failed:
	if(event->event_finit_predefined)
	{
		event_finit_handle predefined_finit = (event_finit_handle)(event->event_finit_predefined->handle);
		assert(predefined_finit != NULL);
		if(predefined_finit(work_event))
			engine_runtime_error("call predefined work event finitializer failed\n");
	}

common_failed:
	if(work_event)
		engine_zfree(ey_engine_work_event_slab(eng), work_event);
	
	return NULL;
}

void ey_runtime_destroy_event(engine_work_event_t *work_event)
{
	assert(work_event != NULL);

	engine_work_t *work = work_event->work;
	assert(work != NULL);

	ey_engine_t *eng = (ey_engine_t*)(work->engine);
	assert(eng != NULL);

	ey_event_t *event = (ey_event_t*)(work_event->event);
	assert(event != NULL);

	/*call userdefined finit function*/
	if(event->event_finit_userdefined)
	{
		event_finit_handle userdefined_finit = (event_finit_handle)(event->event_finit_userdefined->handle);
		assert(userdefined_finit != NULL);
		if(userdefined_finit(work_event))
			engine_runtime_error("call userdefined work event finitializer failed\n");
	}
	
	/*call predefined finit function*/
	if(event->event_finit_predefined)
	{
		event_finit_handle predefined_finit = (event_finit_handle)(event->event_finit_predefined->handle);
		assert(predefined_finit != NULL);
		if(predefined_finit(work_event))
			engine_runtime_error("call predefined work event finitializer failed\n");
	}

	engine_zfree(ey_engine_work_event_slab(eng), work_event);
	return;
}

/*
 * DETECTING :)
 * */
static int do_top_half_detect(engine_work_event_t *work_event)
{
	return 0;
}

static int do_bottom_half_detect(engine_work_event_t *work_event)
{
	return 0;
}

int ey_runtime_detect_event(engine_work_event_t *work_event)
{
	assert(work_event != NULL);

	engine_work_t *engine_work = work_event->work;
	assert(engine_work != NULL);

	ey_engine_t *eng = (ey_engine_t*)(engine_work->engine);
	ey_work_t *work = (ey_work_t*)(engine_work->priv_data);
	assert(eng != NULL && work != NULL);

	ey_event_t *event = (ey_event_t*)(work_event->event);
	engine_action_t *action = work_event->action;
	assert(event != NULL && action != NULL);
	action->action = ENGINE_ACTION_PASS;

	/*
	 * work event detecting is divided into two parts:
	 * 1, top-half will do cluster matching and event enqueue,
	 * 2, bottom-half will do event dequeue and further checking.
	 *
	 * Top-half could be done in other computing unit
	 * such as other core/cpu/gpu/chip, in an async arch
	 * */
	engine_runtime_debug("start to do top half check\n");
	if(do_top_half_detect(work_event))
	{
		engine_runtime_debug("no need to bottom half check, return 0\n");
		return 0;
	}

	engine_runtime_debug("start to do bottom half check\n");
	if(do_bottom_half_detect(work_event))
	{
		engine_runtime_debug("event is clean, return 0\n");
		return 0;
	}
	engine_runtime_debug("event detect return %d\n", action->action==ENGINE_ACTION_PASS?0:-1);
	return 0;
}
