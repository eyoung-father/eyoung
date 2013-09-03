#ifndef ENGINE_MEMORY_H
#define ENGINE_MEMORY_H 1

#include "ey_memory.h"

/*engine*/
#define engine_malloc(sz) ey_malloc(sz)
#define engine_realloc(ptr, sz) ey_realloc(ptr,sz)
#define engine_free(ptr) ey_free(ptr)
#define engine_zalloc(s) ey_zalloc(s)
#define engine_zfree(s,p) ey_zfree(s,p)
#define engine_zclear(s) ey_zclear(s)
#define engine_zfinit(s) ey_zfinit(s)
#define engine_zinit(n,s) ey_zinit((n),(s),NULL)
#define engine_fzalloc(s,z) ey_fzalloc(s,z)
#define engine_fzfree(s,p) ey_fzfree(s,p)
#define engine_fzclear(s) ey_fzclear(s)
#define engine_fzfinit(s) ey_fzfinit(s)
#define engine_fzinit(n,s,m) ey_fzinit((n),(s),(m))


/*lexier*/
#define lexier_malloc(sz) ey_malloc(sz)
#define lexier_realloc(ptr, sz) ey_realloc(ptr,sz)
#define lexier_free(ptr) ey_free(ptr)
#define lexier_zalloc(s) ey_zalloc(s)
#define lexier_zfree(s,p) ey_zfree(s,p)
#define lexier_zclear(s) ey_zclear(s)
#define lexier_zfinit(s) ey_zfinit(s)
#define lexier_zinit(n,s) ey_zinit((n),(s),NULL)

/*parser*/
#define parser_malloc(sz) ey_malloc(sz)
#define parser_realloc(ptr, sz) ey_realloc(ptr,sz)
#define parser_free(ptr) ey_free(ptr)
#define parser_zalloc(s) ey_zalloc(s)
#define parser_zfree(s,p) ey_zfree(s,p)
#define parser_zclear(s) ey_zclear(s)
#define parser_zfinit(s) ey_zfinit(s)
#define parser_zinit(n,s) ey_zinit((n),(s),NULL)
#endif
