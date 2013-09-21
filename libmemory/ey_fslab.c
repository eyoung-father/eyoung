#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ey_queue.h"
#include "ey_lock.h"
#include "ey_memory.h"

typedef struct fslab_head
{
	size_t user_size;
	TAILQ_ENTRY(fslab_head) link;
}fslab_head_t;
typedef TAILQ_HEAD(fslab_head_list, fslab_head) fslab_head_list_t;
#define HEAD2PTR(head) ((void*)((fslab_head_t*)(head)+1))
#define PTR2HEAD(ptr) ((fslab_head_t*)((fslab_head_t*)ptr-1))

struct ey_fslab
{
	ey_slab_t slab;
	fslab_head_list_t malloc_list;
	TAILQ_ENTRY(ey_fslab) link;
	char zname[64];
	memory_handler_t memory_handler;
};
typedef TAILQ_HEAD(ey_fslab_list, ey_fslab) ey_fslab_list_t;
static ey_fslab_list_t fslab_list = TAILQ_HEAD_INITIALIZER(fslab_list);
static ey_spinlock_t fslab_list_lock = EY_SPINLOCK_INITIALIZER;

ey_fslab_t ey_fzinit(char *name, int size, memory_handler_t *handler)
{
	if(size < 0)
	{
		fprintf(stderr, "size %d must be positive\n", size);
		return NULL;
	}

	static memory_handler_t default_handler = {ey_malloc,ey_realloc,ey_free,ey_calloc};
	struct ey_fslab *z = NULL;
	int real_size = size + sizeof(fslab_head_t);

	if(real_size <= 0)
	{
		fprintf(stderr, "size %d is too big\n", size);
		return NULL;
	}

	if(!handler)
		handler = &default_handler;
	
	if(!name)
		name = "unknown";
	
	z = (struct ey_fslab*)handler->malloc(sizeof(struct ey_fslab));
	if(!z)
	{
		fprintf(stderr, "malloc fslab %s failed\n", name);
		return NULL;
	}
	if(size)
		z->slab = ey_zinit(name, real_size, handler);
	else
		z->slab = NULL;
	TAILQ_INIT(&z->malloc_list);
	z->memory_handler = *handler;
	strncpy(z->zname, name, sizeof(z->zname));
	ey_spinlock_lock(&fslab_list_lock);
	TAILQ_INSERT_HEAD(&fslab_list, z, link);
	ey_spinlock_unlock(&fslab_list_lock);
	return z;
}

void *ey_fzalloc(size_t size, struct ey_fslab *z)
{
	size_t real_size = size + sizeof(fslab_head_t);
	fslab_head_t *head = NULL;

	if(!z)
		return NULL;

	if(!z->slab || real_size > ey_zsize(z->slab))
	{
		head = (fslab_head_t*)z->memory_handler.malloc(real_size);
		if(head)
		{
			head->user_size = size;
			TAILQ_INSERT_HEAD(&z->malloc_list, head, link);
			return HEAD2PTR(head);
		}
	}
	else
	{
		head = (fslab_head_t*)ey_zalloc(z->slab);
		if(head)
		{
			head->user_size = size;
			return HEAD2PTR(head);
		}
	}

	return NULL;
}

void ey_fzfree(struct ey_fslab *z, void *item)
{
	if(!z || !item)
		return;
	
	fslab_head_t *head = PTR2HEAD(item);
	size_t real_size = head->user_size + sizeof(fslab_head_t);

	if(!z->slab || real_size>=ey_zsize(z->slab))
	{
		TAILQ_REMOVE(&z->malloc_list, head, link);
		z->memory_handler.free(head);
	}
	else
	{
		ey_zfree(z->slab, head);
	}
}

void ey_fzclear(struct ey_fslab *z)
{
	if(!z)
		return;
	
	if(z->slab)
		ey_zclear(z->slab);
	
	fslab_head_t *to_del = NULL, *tmp = NULL;
	TAILQ_FOREACH_SAFE(to_del, &z->malloc_list, link, tmp)
		z->memory_handler.free(to_del);
}

void ey_fzfinit(struct ey_fslab *z)
{
	if(!z)
		return;
	
	ey_fzclear(z);
	if(z->slab)
		ey_zfinit(z->slab);
	ey_spinlock_lock(&fslab_list_lock);
	TAILQ_REMOVE(&fslab_list, z, link);
	ey_spinlock_unlock(&fslab_list_lock);
	z->memory_handler.free(z);
}
