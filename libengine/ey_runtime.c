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
	ey_assert(eng!=NULL);
	ey_bitmap_t *bitmap = NULL;
	ey_bitmap_t *pp_bitmap = NULL;
	ey_work_t *ey_work = NULL;
	engine_work_t *engine_work = NULL;

	/*alloc bitmap*/
	bitmap = ey_bitmap_create(eng, ey_rhs_id(eng));
	if(!bitmap)
	{
		engine_runtime_error("alloc bitmap failed\n");
		goto common_failed;
	}

	pp_bitmap = ey_bitmap_create(eng, ey_rhs_id(eng));
	if(!pp_bitmap)
	{
		engine_runtime_error("alloc preprocessor bitmap failed\n");
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
	ey_work->preprocessor_bitmap = pp_bitmap;

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
		ey_assert(predefined_init != NULL);
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
		ey_assert(userdefined_init != NULL);
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
		ey_assert(userdefined_finit != NULL);
		if(userdefined_finit(engine_work))
			engine_runtime_error("call userdefined work finalializer failed");
	}
	
predefined_failed:
	/*call predefined engine work finit function*/
	if(ey_work_finit_predefined(eng))
	{
		work_finit_handle predefined_finit = (work_finit_handle)(ey_work_finit_predefined(eng)->handle);
		ey_assert(predefined_finit != NULL);
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

	if(pp_bitmap)
		ey_bitmap_destroy(eng, pp_bitmap);
	return NULL;
}

void ey_runtime_destroy(engine_work_t *work)
{
	ey_assert(work!=NULL && work->engine!=NULL && work->priv_data);
	ey_engine_t *eng = (ey_engine_t*)(work->engine);

	/*remove from work list*/
	ey_spinlock_lock(&ey_engine_lock(eng));
	TAILQ_REMOVE(&ey_engine_work_list(eng), work, link);
	ey_spinlock_unlock(&ey_engine_lock(eng));

	ey_work_t *priv_work = (ey_work_t*)(work->priv_data);
	ey_assert(priv_work!=NULL);

	/*call userdefined engine work finit function*/
	if(ey_work_finit_userdefined(eng))
	{
		work_finit_handle userdefined_finit = (work_finit_handle)(ey_work_finit_userdefined(eng)->handle);
		ey_assert(userdefined_finit != NULL);
		if(userdefined_finit(work))
			engine_runtime_error("call userdefined work finalializer failed");
	}

	/*call predefined engine work finit function*/
	if(ey_work_finit_predefined(eng))
	{
		work_finit_handle predefined_finit = (work_finit_handle)(ey_work_finit_predefined(eng)->handle);
		ey_assert(predefined_finit != NULL);
		if(predefined_finit(work))
			engine_runtime_error("call predefined work finalializer failed");
	}
	
	/*common finit*/
	if(priv_work->local_allocator)
		engine_fzfinit(priv_work->local_allocator);
	
	if(priv_work->state_bitmap)
		ey_bitmap_destroy(eng, priv_work->state_bitmap);
	
	if(priv_work->preprocessor_bitmap)
		ey_bitmap_destroy(eng, priv_work->preprocessor_bitmap);
	
	engine_zfree(ey_work_slab(eng), priv_work);
	engine_zfree(ey_engine_work_slab(eng), work);
	return;
}

engine_work_event_t *ey_runtime_create_event(engine_work_t *work, unsigned long event_id, engine_action_t *action)
{
	ey_assert(action!=NULL);
	action->action = ENGINE_ACTION_PASS;

	ey_assert(work!=NULL && work->engine!=NULL && work->priv_data!=NULL);
	ey_engine_t *eng = (ey_engine_t*)(work->engine);

	ey_assert(event_id <= ey_event_count(eng));
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
		ey_assert(predefined_init != NULL);
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
		ey_assert(userdefined_init != NULL);
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
		ey_assert(userdefined_finit != NULL);
		if(userdefined_finit(work_event))
			engine_runtime_error("call userdefined work event finitializer failed\n");
	}

predefined_failed:
	if(event->event_finit_predefined)
	{
		event_finit_handle predefined_finit = (event_finit_handle)(event->event_finit_predefined->handle);
		ey_assert(predefined_finit != NULL);
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
	ey_assert(work_event != NULL);

	engine_work_t *work = work_event->work;
	ey_assert(work != NULL);

	ey_engine_t *eng = (ey_engine_t*)(work->engine);
	ey_assert(eng != NULL);

	ey_event_t *event = (ey_event_t*)(work_event->event);
	ey_assert(event != NULL);

	/*call userdefined finit function*/
	if(event->event_finit_userdefined)
	{
		event_finit_handle userdefined_finit = (event_finit_handle)(event->event_finit_userdefined->handle);
		ey_assert(userdefined_finit != NULL);
		if(userdefined_finit(work_event))
			engine_runtime_error("call userdefined work event finitializer failed\n");
	}
	
	/*call predefined finit function*/
	if(event->event_finit_predefined)
	{
		event_finit_handle predefined_finit = (event_finit_handle)(event->event_finit_predefined->handle);
		ey_assert(predefined_finit != NULL);
		if(predefined_finit(work_event))
			engine_runtime_error("call predefined work event finitializer failed\n");
	}

	engine_zfree(ey_engine_work_event_slab(eng), work_event);
	return;
}

/*
 * DETECTING :)
 * */
static int do_bottom_half_detect(engine_work_event_t *work_event)
{
	engine_work_t *engine_work = work_event->work;
	ey_engine_t *eng = (ey_engine_t*)(engine_work->engine);
	ey_event_t *event = (ey_event_t*)(work_event->event);
	ey_work_t *work = (ey_work_t*)(engine_work->priv_data);
	ey_bitmap_t *bitmap = work->state_bitmap;
	ey_bitmap_t *pp_bitmap = work->preprocessor_bitmap;
	ey_rhs_item_t *rhs_item = NULL;
	int prefix = 0, postfix=0;
	event_condition_handle condition_cb = NULL;
	event_action_handle action_cb = NULL;
	
	/*check uncluster rhs_items first*/
	TAILQ_FOREACH(rhs_item, &event->item_list, event_link)
	{
		prefix = ey_prefix_array(eng)[rhs_item->rhs_id];
		postfix = ey_postfix_array(eng)[rhs_item->rhs_id];
		
		if(rhs_item->clustered ^ ey_bitmap_isset(eng, pp_bitmap, rhs_item->rhs_id))
		{
			engine_runtime_debug("%lu:%lu:%lu not match preprocessor, skip it\n", 
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			continue;
		}
		
		if(prefix && !ey_bitmap_isset(eng, bitmap, rhs_item->rhs_id))
		{
			engine_runtime_debug("%lu:%lu:%lu prefix is not satisfied, skip it\n",
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			continue;
		}

		if(rhs_item->condition && rhs_item->condition->addr)
		{
			condition_cb = (event_condition_handle)(rhs_item->condition->addr);
			if(condition_cb(engine_work, work_event) <= 0)
			{
				engine_runtime_debug("%lu:%lu:%lu condition check return false, skip it\n",
					rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
				continue;
			}
		}

		if(rhs_item->action && rhs_item->action->addr)
		{
			action_cb = (event_action_handle)(rhs_item->action->addr);
			if(action_cb(engine_work, work_event) <= 0)
			{
				engine_runtime_debug("%lu:%lu:%lu action check return false, skip it\n",
					rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
				continue;
			}
		}
		
		ey_bitmap_set(eng, bitmap, rhs_item->rhs_id);
		if(postfix)
		{
			engine_runtime_debug("%lu:%lu:%lu need further check later\n",
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			continue;
		}
		else
		{
			ey_rhs_item_t *prev = rhs_item;
			while(prev)
			{
				ey_bitmap_clear(eng, bitmap, prev->rhs_id);
				ey_bitmap_clear(eng, pp_bitmap, prev->rhs_id);
				prev = TAILQ_PREV(prev, ey_rhs_item_list, link);
			}
			goto find_something;
		}
	}

	engine_runtime_debug("do_bottom_half_detect find nothing and return 1\n");
	return 1;

find_something:
	engine_runtime_debug("%lu:%lu:%lu do_bottom_half_detect find something and return 0\n",
		rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
	return 0;
}

int ey_runtime_detect_event(engine_work_event_t *work_event)
{
	ey_assert(work_event != NULL);

	engine_work_t *engine_work = work_event->work;
	ey_assert(engine_work != NULL);

	ey_work_t *work = (ey_work_t*)(engine_work->priv_data);
	ey_assert(work != NULL);

	ey_event_t *event = (ey_event_t*)(work_event->event);
	engine_action_t *action = work_event->action;
	ey_assert(event != NULL && action != NULL);
	action->action = ENGINE_ACTION_PASS;
	engine_runtime_debug("check event %s\n", event->name);
	
	/*call predefined preprocessor function*/
	if(event->event_preprocessor_predefined)
	{
		event_preprocess_handle predefined_preprocessor = (event_preprocess_handle)(event->event_preprocessor_predefined->handle);
		ey_assert(predefined_preprocessor != NULL);
		if(predefined_preprocessor(work_event))
		{
			engine_runtime_error("call predefined work event preprocessor failed\n");
			return 0;
		}
	}

	/*call userdefined preprocessor function*/
	if(event->event_preprocessor_userdefined)
	{
		event_preprocess_handle userdefined_preprocessor = (event_preprocess_handle)(event->event_preprocessor_userdefined->handle);
		ey_assert(userdefined_preprocessor != NULL);
		if(userdefined_preprocessor(work_event))
		{
			engine_runtime_error("call userdefined work event preprocessor failed\n");
			return 0;
		}
	}

	engine_runtime_debug("start to do bottom half check\n");
	if(do_bottom_half_detect(work_event))
		engine_runtime_debug("event is clean, return 0\n");

	engine_runtime_debug("event detect return %d\n", action->action==ENGINE_ACTION_PASS?0:-1);
	return action->action==ENGINE_ACTION_PASS?0:-1;
}
