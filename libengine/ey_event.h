#ifndef EY_EVENT_H
#define EY_EVENT_H 1

#include "ey_queue.h"
#include "ey_signature.h"
#include "ey_loc.h"
#include "libmatch.h"
#include "libengine_type.h"

typedef struct ey_event
{
	ey_location_t location;
	unsigned long event_id;
	char *name;
	char *define;

	ey_acsm_t cluster_pattern;
	ey_rhs_item_list_t cluster_item_list;
	ey_rhs_item_list_t uncluster_item_list;

	ey_code_t *event_init_predefined;
	ey_code_t *event_finit_predefined;
	ey_code_t *event_init_userdefined;
	ey_code_t *event_finit_userdefined;
}ey_event_t;

struct ey_engine;

extern ey_event_t *ey_alloc_event(struct ey_engine *eng, ey_location_t *location, char *name, char *define);
extern void ey_free_event(struct ey_engine *eng, ey_event_t *event);
extern ey_event_t *ey_find_event(struct ey_engine *eng, const char *name);
extern int ey_event_init(struct ey_engine *eng);
extern void ey_event_finit(struct ey_engine *eng);

extern int ey_event_set_runtime_init(struct ey_engine *eng, const char *event_name, int user_define,
	const char *function, event_init_handle address, ey_location_t *location);
extern int ey_event_set_runtime_finit(struct ey_engine *eng, const char *event_name, int user_define,
	const char *function, event_finit_handle address, ey_location_t *location);
#endif
