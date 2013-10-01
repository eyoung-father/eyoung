#ifndef EY_WORK_H
#define EY_WORK_H 1

#include "ey_loc.h"
#include "libengine_type.h"
struct ey_engine;

extern int ey_work_set_init(struct ey_engine *eng, int user_define, 
	const char *function, work_init_handle address, ey_location_t *loc);
extern int ey_work_set_finit(struct ey_engine *eng, int user_define, 
	const char *function, work_finit_handle address, ey_location_t *loc);
#endif
