#ifndef HTML_MEM_H
#define HTML_MEM_H 1

#include <stdlib.h>
#include "ey_memory.h"

/*KMALLOC/KFREE*/
#define html_malloc(sz) ey_malloc(sz)
#define html_realloc(ptr,sz) ey_realloc(ptr,sz)
#define html_free(p) ey_free(p)

/*for slab*/
#define html_zalloc(slab) ey_zalloc(slab)
#define html_zfree(slab,p) ey_zfree(slab,p)
#define html_zclear(slab) ey_zclear(slab)
#define html_zfinit(slab) ey_zfinit(slab)
#define html_zinit(name,size) ey_zinit((name),(size),NULL)

#define html_fzalloc(size,slab) ey_fzalloc(size,slab)
#define html_fzfree(slab,p) ey_fzfree(slab,p)
#define html_fzclear(slab) ey_fzclear(slab)
#define html_fzfinit(slab) ey_fzfinit(slab)
#define html_fzinit(name,size) ey_fzinit((name),(size),NULL)

#endif
