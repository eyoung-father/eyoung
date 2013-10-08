#ifndef EY_WORK_H
#define EY_WORK_H 1

#include "ey_loc.h"
#include "libengine_type.h"
#include "ey_bitmap.h"
#include "ey_memory.h"
#include "ey_lock.h"
struct ey_engine;

typedef struct ey_work
{
	ey_bitmap_t *state_bitmap;
	ey_fslab_t local_allocator;

	engine_work_event_list_t event_list;
	ey_spinlock_t work_lock;
	int lock_index;
}ey_work_t;

extern int ey_work_init(struct ey_engine *eng);
extern void ey_work_finit(struct ey_engine *eng);
extern int ey_work_set_runtime_init(struct ey_engine *eng, int user_define, 
	const char *function, work_init_handle address, ey_location_t *loc);
extern int ey_work_set_runtime_finit(struct ey_engine *eng, int user_define, 
	const char *function, work_finit_handle address, ey_location_t *loc);
#endif
