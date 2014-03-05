#ifndef EY_PREPROCESSOR_H
#define EY_PREPROCESSOR_H 1

#include "libengine_type.h"
#include "ey_engine.h"

struct ey_engine;
struct ey_preprocessor;

typedef int (*preprocessor_load_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor, const char *signature, unsigned long signature_id);
typedef int (*preprocessor_detect_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor, engine_work_event_t *work_event);
typedef void (*preprocessor_finit_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor);
typedef int (*preprocessor_init_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor);

typedef struct ey_preprocessor
{
	TAILQ_ENTRY(ey_preprocessor) link;

	char name[64];
	void *processor_data;
	
	preprocessor_init_fn preprocessor_init;
	preprocessor_load_fn preprocessor_load;
	preprocessor_detect_fn preprocessor_detect;
	preprocessor_finit_fn preprocessor_finit;
}ey_preprocessor_t;
typedef TAILQ_HEAD(ey_preprocessor_list, ey_preprocessor) ey_preprocessor_list_t;

extern ey_preprocessor_t *ey_preprocessor_register(struct ey_engine *engine, const char *name, 
	preprocessor_init_fn init, preprocessor_load_fn load,
	preprocessor_detect_fn detect, preprocessor_finit_fn finit);

extern int ey_preprocessor_init(struct ey_engine *engine);
extern void ey_preprocessor_finit(struct ey_engine *engine);

#endif
