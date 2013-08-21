#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ey_queue.h"
#include "ey_lock.h"
#include "ey_memory.h"

#define PAGE_SIZE 4096
#define MIN_PAGE_ITEMS 4
#define ITEM_ALLOCTED 0x0001

typedef struct slab_item_head 
{
	unsigned short offset;
	unsigned short flag;
	SLIST_ENTRY(slab_item_head) link;
}slab_item_head_t;
typedef SLIST_HEAD(slab_item_head_list, slab_item_head) slab_item_head_list_t;
#define HEAD2PTR(head) ((void*)((slab_item_head_t*)(head)+1))
#define PTR2HEAD(ptr) ((slab_item_head_t*)((slab_item_head_t*)ptr-1))

typedef struct slab_item_tail
{
	unsigned long magic;
}slab_item_tail_t;
#define HEAD2TAIL(h,z) ((slab_item_tail_t*)((void*)(h)+sizeof(slab_item_head_t)+(z)->item_real_size))
#define PTR2TAIL(p,z) HEAD2TAIL(PTR2HEAD(p),(z))
#define HEAD2MAGIC(h,z) (HEAD2TAIL((h),(z))->magic)
#define PTR2MAGIC(p,z) (PTR2TAIL((h),(z))->magic)

typedef struct slab_page
{
	unsigned short free_count;
	TAILQ_ENTRY(slab_page) link;
	slab_item_head_list_t free_item;
	slab_item_head_t items[0];
}slab_page_t;
typedef TAILQ_HEAD(slab_page_list, slab_page) slab_page_list_t;
#define HEAD2PAGE(h) ((slab_page_t*)((void*)(h)-(h)->offset))
#define PTR2PAGE(p) HEAD2PAGE(PTR2HEAD(p))

struct ey_slab 
{
	slab_page_list_t full_page_list;
	slab_page_list_t alloc_page_list;
	ey_spinlock_t lock;

	unsigned int page_count;
	unsigned int alloc_item_count;
	unsigned short item_real_size;
	unsigned short item_count_per_page;
	memory_handler_t memory_handler;

	char zname[64];
	TAILQ_ENTRY(ey_slab) link;
};
typedef TAILQ_HEAD(ey_slab_list, ey_slab) ey_slab_list_t;
static ey_slab_list_t slab_list = TAILQ_HEAD_INITIALIZER(slab_list);
static ey_spinlock_t slab_list_lock = EY_SPINLOCK_INITIALIZER;

static int get_page(struct ey_slab *z)
{
	slab_page_t *page = NULL;
	int index = 0;
	slab_item_head_t *head = NULL;
	slab_item_tail_t *tail = NULL;
	void *p = NULL;
	int step = sizeof(slab_item_head_t) + z->item_real_size + sizeof(slab_item_tail_t);

	if((page = (slab_page_t *)z->memory_handler.malloc(PAGE_SIZE)) == NULL)
		return 1;

	for(index=0, p=page->items; index<z->item_count_per_page; index++, p+=step)
	{
		head = (slab_item_head_t*)p;
		tail = HEAD2TAIL(head,z);
		head->offset = (unsigned long)p - (unsigned long)page;
		head->flag = 0;
		tail->magic = MEM_MAGIC;
		SLIST_INSERT_HEAD(&page->free_item, head, link);
	}
	page->free_count = z->item_count_per_page;
	z->page_count++;
	TAILQ_INSERT_HEAD(&z->alloc_page_list, page, link);
	return 0;
}

void *ey_zalloc(struct ey_slab *z)
{
	slab_item_head_t *head = NULL;

	ey_spinlock_lock(&z->lock);
retry:
	if(!TAILQ_EMPTY(&z->alloc_page_list))
	{
		slab_page_t *page = TAILQ_FIRST(&z->alloc_page_list);
		head = SLIST_FIRST(&page->free_item);
		SLIST_REMOVE_HEAD(&page->free_item, link);
		page->free_count--;
		z->alloc_item_count++;

		if(page->free_count == 0)
		{
			TAILQ_REMOVE(&z->alloc_page_list, page, link);
			TAILQ_INSERT_HEAD(&z->full_page_list, page, link);
		}
		ey_spinlock_unlock(&z->lock);
	}
	else
	{
		if(get_page(z))
		{
			ey_spinlock_unlock(&z->lock);
			return NULL;
		}
		goto retry;
	}
	if(head->flag & ITEM_ALLOCTED)
	{
		fprintf(stderr, "item has been allocated\n");
		*(int*)0 = 0;
	}

	head->flag |= ITEM_ALLOCTED;
	return HEAD2PTR(head);
}

void ey_zfree(struct ey_slab *z, void *item)
{
	if(item == NULL)
		return;

	slab_item_head_t *head = PTR2HEAD(item);
	slab_page_t *page = HEAD2PAGE(head);
	unsigned long magic = HEAD2MAGIC(head,z);

	if(magic != MEM_MAGIC)
	{
		fprintf(stderr, "Slab memory overwrite\n");
		*(int*)0 = 0;
	}

	if(!(head->flag & ITEM_ALLOCTED))
	{
		fprintf(stderr, "Slab memory double free\n");
		*(int*)0 = 0;
	}

	ey_spinlock_lock(&z->lock);

	z->alloc_item_count--;
	if((page->free_count + 1) == z->item_count_per_page)
	{
		TAILQ_REMOVE(&z->alloc_page_list, page, link);
		z->page_count--;
		z->memory_handler.free(page);
		ey_spinlock_unlock(&z->lock);
		return;
	}

	head->flag &= ~ITEM_ALLOCTED;
	SLIST_INSERT_HEAD(&page->free_item, head, link);
	page->free_count++;

	if(page->free_count == 1)
	{
		TAILQ_REMOVE(&z->full_page_list, page, link);
		TAILQ_INSERT_TAIL(&z->alloc_page_list, page, link);
	}
	ey_spinlock_unlock(&z->lock);
	return;
}

ey_slab_t ey_zinit(char *name, int size, memory_handler_t *handler)
{
	if(size <= 0)
	{
		fprintf(stderr, "size %d must be positive\n", size);
		return NULL;
	}
	static memory_handler_t default_handler = {ey_malloc,ey_realloc,ey_free};
	struct ey_slab *z = NULL;
	int real_size = REAL_SIZE(size);
	int total_size = real_size + sizeof(slab_item_head_t) + sizeof(slab_item_tail_t);

	if(!handler)
		handler = &default_handler;

	if((real_size<=0) || (total_size*MIN_PAGE_ITEMS+sizeof(slab_page_t) > PAGE_SIZE))
	{
		fprintf(stderr, "size %d of %s is too big\n", size, name);
		return NULL;
	}

	z = (struct ey_slab*)handler->malloc(sizeof(struct ey_slab));
	if(z == NULL)
	{
		fprintf(stderr, "malloc slab %s failed\n", name);
		return NULL;
	}

	z->item_real_size = real_size;
	ey_spinlock_init(&z->lock);
	TAILQ_INIT(&z->full_page_list);
	TAILQ_INIT(&z->alloc_page_list);
	z->page_count = 0;
	z->alloc_item_count = 0;
	z->item_count_per_page = (PAGE_SIZE-sizeof(slab_page_t))/total_size;
	z->memory_handler = *handler;
	strncpy(z->zname, name?name:"", sizeof(z->zname)-1);
	ey_spinlock_lock(&slab_list_lock);
	TAILQ_INSERT_HEAD(&slab_list, z, link);
	ey_spinlock_unlock(&slab_list_lock);
	return z;
}

void ey_zclear(struct ey_slab *z)
{
	slab_page_t *page = NULL, *tmp = NULL;
	
	ey_spinlock_lock(&z->lock);
	TAILQ_FOREACH_SAFE(page, &z->full_page_list, link, tmp)
		z->memory_handler.free(page);
	TAILQ_FOREACH_SAFE(page, &z->alloc_page_list, link, tmp)
		z->memory_handler.free(page);
	TAILQ_INIT(&z->full_page_list);
	TAILQ_INIT(&z->alloc_page_list);
	z->page_count = z->alloc_item_count = 0;
	ey_spinlock_unlock(&z->lock);
}

void ey_zfinit(struct ey_slab *z)
{
	ey_spinlock_lock(&slab_list_lock);
	TAILQ_REMOVE(&slab_list, z, link);
	ey_spinlock_unlock(&slab_list_lock);
	ey_zclear(z);
	z->memory_handler.free(z);
}

size_t ey_zsize(struct ey_slab *z)
{
	return z?z->item_real_size:0;
}

#ifdef SLAB_MAIN
typedef struct test
{
	char buf[512];
	int buf_len;
}test_t;
int main(int argc, char*argv[])
{
	ey_slab_t slab = ey_zinit("test", sizeof(test_t), NULL);
	if(!slab)
	{
		fprintf(stderr, "init failed\n");
		return -1;
	}

	test_t *array[8] = {NULL}, *tmp = NULL;
	int index;
	for(index=0; index<sizeof(array)/sizeof(array[0]); index++)
	{
		tmp = array[index] = ey_zalloc(slab);
		if(!tmp)
		{
			fprintf(stderr, "alloc index %d failed\n", index);
			goto failed;
		}
		memset(tmp->buf, index, sizeof(tmp->buf));
		tmp->buf_len = sizeof(tmp->buf);
	}

failed:
	for(index=0; array[index] && index<sizeof(array)/sizeof(array[0]); index++)
	{
		ey_zfree(slab, array[index]);
		array[index] = NULL;
	}
	ey_zfinit(slab);
	return 0;
}
#endif
