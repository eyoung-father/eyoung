#ifndef EY_RUNTIME_H
#define EY_RUNTIME_H 1

#include "libengine_type.h"

#define MAX_RUNTIME_ITEM	(1<<10)

struct ey_engine;
extern int ey_runtime_init(struct ey_engine *eng);
extern void ey_runtime_finit(struct ey_engine *eng);

extern engine_work_t* ey_runtime_create(struct ey_engine *eng);
extern void ey_runtime_destroy(engine_work_t *work);
extern engine_work_event_t *ey_runtime_create_event(engine_work_t *work, unsigned long event_id, engine_action_t *action);
extern int ey_runtime_detect_event(engine_work_event_t *event);
extern void ey_runtime_destroy_event(engine_work_event_t *event);
#endif
