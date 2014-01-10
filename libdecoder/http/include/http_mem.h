#ifndef HTTP_MEM_H
#define HTTP_MEM_H 1

#include <stdlib.h>
#include "ey_memory.h"

/*KMALLOC/KFREE*/
#define http_malloc(sz) ey_malloc(sz)
#define http_realloc(ptr,sz) ey_realloc(ptr,sz)
#define http_free(p) ey_free(p)

/*for slab*/
#define http_zalloc(slab) ey_zalloc(slab)
#define http_zfree(slab,p) ey_zfree(slab,p)
#define http_zclear(slab) ey_zclear(slab)
#define http_zfinit(slab) ey_zfinit(slab)
#define http_zinit(name,size) ey_zinit((name),(size),NULL)

#endif
