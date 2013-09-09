#ifndef EY_EVENT_H
#define EY_EVENT_H 1

#include "ey_loc.h"
typedef struct ey_event
{
	ey_location_t location;
	char *name;
	char *define;
}ey_event_t;

typedef char* ey_event_key_t;
struct ey_engine;

extern ey_event_t *ey_alloc_event(struct ey_engine *eng, ey_location_t *location, char *name, char *define);
extern void ey_free_event(struct ey_engine *eng, ey_event_t *event);
extern ey_event_t *ey_define_event(struct ey_engine *eng, ey_event_t *event);
extern ey_event_t *ey_find_event(struct ey_engine *eng, char *name);
extern int ey_event_init(struct ey_engine *eng);
extern void ey_event_finit(struct ey_engine *eng);
#endif