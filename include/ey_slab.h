#ifndef EY_SLAB_H
#define EY_SLAB_H 1

struct memory_handler;
struct ey_slab;
typedef struct ey_slab *ey_slab_t;

extern ey_slab_t ey_zinit(char *name, int size, struct memory_handler *mem);
extern void *ey_zalloc(ey_slab_t z);
extern void ey_zfree(ey_slab_t z, void *item);
extern void ey_zfinit(ey_slab_t z);
extern void ey_zclear(ey_slab_t z);
extern size_t ey_zsize(ey_slab_t z);
#endif
