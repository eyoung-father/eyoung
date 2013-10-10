#ifndef EY_FSLAB_H
#define EY_FSLAB_H 1

struct memory_handler;
struct ey_fslab;
typedef struct ey_fslab *ey_fslab_t;

extern ey_fslab_t ey_fzinit(char *name, int size, struct memory_handler *mem);
extern void *ey_fzalloc(size_t size, ey_fslab_t z);
extern void *ey_fzrealloc(void *old_item, size_t new_size, ey_fslab_t z);
extern void ey_fzfree(ey_fslab_t z, void *item);
extern void ey_fzfinit(ey_fslab_t z);
extern void ey_fzclear(ey_fslab_t z);
#endif
