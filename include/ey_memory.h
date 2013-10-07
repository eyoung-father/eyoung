#ifndef EY_MEMORY_H
#define EY_MEMORY_H 1

#define LOCAL_MEMORY __thread

#define	MEM_ROUNDING (sizeof(unsigned long))
#define REAL_SIZE(_size)   ((_size + MEM_ROUNDING - 1) & ~(MEM_ROUNDING - 1))
#define MEM_MAGIC (unsigned long)0xdeaddeaddeaddeadUL

typedef void* (*malloc_fn)(size_t size);
typedef void* (*realloc_fn)(void *old, size_t new_size);
typedef void (*free_fn)(void *ptr);
typedef void* (*calloc_fn)(size_t nmemb, size_t size);

typedef struct memory_handler
{
	malloc_fn malloc;
	realloc_fn realloc;
	free_fn free;
	calloc_fn calloc;
}memory_handler_t;

#include "ey_slab.h"
#include "ey_malloc.h"
#include "ey_fslab.h"
#include "ey_hash.h"

#endif
