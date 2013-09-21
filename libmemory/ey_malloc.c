#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ey_memory.h"

typedef struct malloc_head
{
	size_t real_size;
}malloc_head_t;
#define HEAD2PTR(head) ((void*)((malloc_head_t*)(head)+1))
#define PTR2HEAD(ptr) ((malloc_head_t*)((malloc_head_t*)ptr-1))

typedef struct malloc_tail
{
	unsigned long magic;
}malloc_tail_t;
#define HEAD2TAIL(h) ((malloc_tail_t*)((void*)(h)+sizeof(malloc_head_t)+(h)->real_size))
#define PTR2TAIL(p) HEAD2TAIL(PTR2HEAD(p))
#define HEAD2MAGIC(h) (HEAD2TAIL(h)->magic)
#define PTR2MAGIC(p) (PTR2TAIL(h)->magic)

void *ey_malloc(size_t size)
{
	if(!size)
		return NULL;
	
	size_t real_size = REAL_SIZE(size);
	malloc_head_t *ptr = (malloc_head_t*)malloc(sizeof(malloc_head_t) + real_size + sizeof(malloc_tail_t));
	if(ptr)
	{
		ptr->real_size = real_size;
		HEAD2TAIL(ptr)->magic = MEM_MAGIC;
	}

	return HEAD2PTR(ptr);
}

void *ey_calloc(size_t nmemb, size_t size)
{
	if(!size)
		return NULL;
	
	size_t real_size = REAL_SIZE(size*nmemb);
	malloc_head_t *ptr = (malloc_head_t*)malloc(sizeof(malloc_head_t) + real_size + sizeof(malloc_tail_t));
	if(ptr)
	{
		ptr->real_size = real_size;
		HEAD2TAIL(ptr)->magic = MEM_MAGIC;
	}
	return memset(HEAD2PTR(ptr), 0, real_size);
}

void ey_free(void* ptr)
{
	if(!ptr)
		return;
	
	if(PTR2TAIL(ptr)->magic != MEM_MAGIC)
	{
		fprintf(stderr, "memory overwrite, magic %lu\n", PTR2TAIL(ptr)->magic);
		*(int*)0 = 0;
	}
	free(PTR2HEAD(ptr));
}

void *ey_realloc(void *ptr, size_t new_size)
{
	if(!ptr)
		return ey_malloc(new_size);

	malloc_head_t *old_head = PTR2HEAD(ptr);
	size_t old_real_size = old_head->real_size;
	size_t new_real_size = REAL_SIZE(new_size);

	if(old_real_size >= new_real_size)
		return ptr;
	
	void *ret = ey_malloc(new_size);
	if(ret)
	{
		memcpy(ret, ptr, old_real_size);
		ey_free(ptr);
	}

	return ret;
}
