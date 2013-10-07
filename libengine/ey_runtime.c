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
	return NULL;
}

void ey_runtime_destroy_event(engine_work_event_t *event)
{
	return;
}

int ey_runtime_detect_event(engine_work_event_t *event)
{
	return 0;
}
