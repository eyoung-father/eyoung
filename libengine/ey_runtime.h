#ifndef EY_RUNTIME_H
#define EY_RUNTIME_H 1

#include "libengine_type.h"

struct ey_engine;
extern int ey_runtime_init(struct ey_engine *eng);
extern void ey_runtime_finit(struct ey_engine *eng);

extern engine_work_t* ey_runtime_create(struct ey_engine *eng);
extern void ey_runtime_destroy(engine_work_t *work);
extern int ey_runtime_detect(engine_work_event_t *event);
#endif
