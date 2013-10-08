#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "ey_engine.h"
#include "ey_runtime.h"

typedef struct ey_runtime_item
{
	unsigned long head;
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
	static int lock_index;

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
	lock_index++;
	if(lock_index >= MAX_RUNTIME_ITEM)
		lock_index = 0;
	ey_spinlock_unlock(&ey_engine_lock(eng));
	ey_work->lock_index = lock_index;
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
static int acsm_match_cb(void *id, void *tree, int index, void *data, void *neg_list)
{
	engine_work_event_t *work_event = (engine_work_event_t*)data;
	engine_work_t *engine_work = work_event->work;
	ey_work_t *work = (ey_work_t*)(engine_work->priv_data);
	ey_runtime_item_t *item = runtime_item + work->lock_index;
	unsigned long rhs_id = (unsigned long)id;
	
	if(debug_engine_runtime)
	{
		ey_engine_t *eng = (ey_engine_t*)(engine_work->engine);
		ey_rhs_item_t *rhs_item = ey_hash_find(ey_rhs_item_hash(eng), id);
		assert(rhs_item!=NULL);
		engine_runtime_debug("find rhs item %lu:%lu:%lu\n",
			rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
	}
	item->results[rhs_id] = item->head;
	item->head = rhs_id;
	return 0;
}

static int do_top_half_detect(engine_work_event_t *work_event)
{
	engine_work_t *engine_work = work_event->work;
	ey_event_t *event = (ey_event_t*)(work_event->event);
	ey_work_t *work = (ey_work_t*)(engine_work->priv_data);
	ey_runtime_item_t *item = runtime_item + work->lock_index;

	if(TAILQ_EMPTY(&event->cluster_item_list) && TAILQ_EMPTY(&event->uncluster_item_list))
	{
		engine_runtime_debug("empty cluster and uncluster item list, no need to further checking\n");
		return 1;
	}

	if(TAILQ_EMPTY(&event->uncluster_item_list) && (!work_event->data || !work_event->data_len))
	{
		engine_runtime_debug("only need do cluster check, but cluster matching data is not set\n");
		return 1;
	}

	item->head = (unsigned long)-1;
	if(event->cluster_pattern && work_event->data && work_event->data_len)
	{
		int last_state = 0;
		ey_acsm_search(event->cluster_pattern, work_event->data, work_event->data_len, acsm_match_cb, work_event, &last_state);
	}
	engine_runtime_debug("need do further check, do_top_half_detect return 0\n");
	return 0;
}

static int do_bottom_half_detect(engine_work_event_t *work_event)
{
	engine_work_t *engine_work = work_event->work;
	ey_engine_t *eng = (ey_engine_t*)(engine_work->engine);
	ey_event_t *event = (ey_event_t*)(work_event->event);
	ey_work_t *work = (ey_work_t*)(engine_work->priv_data);
	ey_bitmap_t *bitmap = work->state_bitmap;
	ey_runtime_item_t *item = runtime_item + work->lock_index;
	ey_rhs_item_t *rhs_item = NULL;
	int prefix = 0, postfix=0;
	event_condition_handle condition_cb = NULL;
	event_action_handle action_cb = NULL;
	unsigned long next_id = 0;
	
	/*check uncluster rhs_items first*/
	TAILQ_FOREACH(rhs_item, &event->uncluster_item_list, event_link)
	{
		prefix = ey_prefix_array(eng)[rhs_item->rhs_id];
		postfix = ey_postfix_array(eng)[rhs_item->rhs_id];

		if(item->results[rhs_item->rhs_id])
		{
			engine_runtime_debug("%lu:%lu:%lu match negative, skip it\n", 
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

		if(postfix)
		{
			if(prefix)
				ey_bitmap_clear(eng, bitmap, prefix);
			ey_bitmap_set(eng, bitmap, postfix);
			engine_runtime_debug("%lu:%lu:%lu need further check later\n",
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			continue;
		}
		else
		{
			if(prefix)
				ey_bitmap_clear(eng, bitmap, prefix);
			goto find_something;
		}
	}
	
	/*do cluster rhs_item checking*/
	while(item->head != (unsigned long)-1)
	{
		rhs_item = ey_hash_find(ey_rhs_item_hash(eng), (void*)&item->head);
		assert(rhs_item!=NULL);

		if(!rhs_item->clustered)
		{
			engine_runtime_debug("%lu:%lu:%lu is not in cluster list, skip it\n",
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			goto do_next;
		}

		prefix = ey_prefix_array(eng)[rhs_item->rhs_id];
		postfix = ey_postfix_array(eng)[rhs_item->rhs_id];

		if(prefix && !ey_bitmap_isset(eng, bitmap, rhs_item->rhs_id))
		{
			engine_runtime_debug("%lu:%lu:%lu prefix is not satisfied, skip it\n",
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			goto do_next;
		}

		if(rhs_item->condition && rhs_item->condition->addr)
		{
			condition_cb = (event_condition_handle)(rhs_item->condition->addr);
			if(condition_cb(engine_work, work_event) <= 0)
			{
				engine_runtime_debug("%lu:%lu:%lu condition check return false, skip it\n",
					rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
				goto do_next;
			}
		}

		if(rhs_item->action && rhs_item->action->addr)
		{
			action_cb = (event_action_handle)(rhs_item->action->addr);
			if(action_cb(engine_work, work_event) <= 0)
			{
				engine_runtime_debug("%lu:%lu:%lu action check return false, skip it\n",
					rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
				goto do_next;
			}
		}

		if(postfix)
		{
			if(prefix)
				ey_bitmap_clear(eng, bitmap, prefix);
			ey_bitmap_set(eng, bitmap, postfix);
			engine_runtime_debug("%lu:%lu:%lu need further check later\n",
				rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
			goto do_next;
		}
		else
		{
			if(prefix)
				ey_bitmap_clear(eng, bitmap, prefix);
			goto find_something;
		}

do_next:
		next_id = item->results[item->head];
		item->results[item->head] = 0;
		item->head = next_id;
		continue;
	}

	engine_runtime_debug("do_bottom_half_detect find nothing and return 1\n");
	return 1;

find_something:
	engine_runtime_debug("%lu:%lu:%lu do_bottom_half_detect find something and return 0\n",
		rhs_item->signature_id, rhs_item->rhs_signature_position, rhs_item->rhs_item_position);
	
	/*reset item->result*/
	while(item->head != (unsigned long)-1)
	{
		next_id = item->results[item->head];
		item->results[item->head] = 0;
		item->head = next_id;
	}
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

	int lock_index = work->lock_index;
	ey_runtime_item_t *item = runtime_item + lock_index;
	assert(lock_index>=0 && lock_index<MAX_RUNTIME_ITEM);
	/*
	 * work event detecting is divided into two parts:
	 * 1, top-half will do cluster matching and event enqueue,
	 * 2, bottom-half will do event dequeue and further checking.
	 *
	 * Top-half could be done in other computing unit
	 * such as other core/cpu/gpu/chip, in an async arch
	 * */
	engine_runtime_debug("start to do top half check\n");
	TAILQ_INSERT_TAIL(&work->event_list, work_event, link);
	int event_count = 0;
	engine_work_event_t *tmp = NULL;

	ey_spinlock_lock(&item->lock);
	TAILQ_FOREACH_SAFE(work_event, &work->event_list, link, tmp)
	{
		if(event_count++ >= MAX_RUNTIME_EVENT)
		{
			engine_runtime_debug("check too many event once\n");
			break;
		}

		if(do_top_half_detect(work_event))
		{
			ey_spinlock_unlock(&item->lock);
			engine_runtime_debug("no need to bottom half check, return 0\n");
			return 0;
		}

		engine_runtime_debug("start to do bottom half check\n");
		if(do_bottom_half_detect(work_event))
		{
			ey_spinlock_unlock(&item->lock);
			engine_runtime_debug("event is clean, return 0\n");
			return 0;
		}

		TAILQ_REMOVE(&work->event_list, work_event, link);
		if(work->from_signature)
			ey_runtime_destroy_event(work_event);
	}
	ey_spinlock_unlock(&item->lock);
	engine_runtime_debug("event detect return %d\n", action->action==ENGINE_ACTION_PASS?0:-1);
	return 0;
}
