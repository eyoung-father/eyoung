#ifndef EY_MALLOC
#define EY_MALLOC 1

#include "ey_memory.h"

extern void* ey_malloc(size_t size);
extern void* ey_realloc(void *old, size_t new_size);
extern void ey_free(void *ptr);
#endif
