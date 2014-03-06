#ifndef EY_PREPROCESSOR_H
#define EY_PREPROCESSOR_H 1

#include "libengine_type.h"
#include "ey_engine.h"

struct ey_engine;
struct ey_preprocessor;

typedef int (*preprocessor_load_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor, const char *signature, unsigned long signature_id);
typedef int (*preprocessor_load_finish_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor);

typedef int (*preprocessor_detect_init_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor, engine_work_t *work);
typedef int (*preprocessor_detect_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor, engine_work_t *work, const char *buf, size_t buf_len, int from_client);
typedef void (*preprocessor_detect_finit_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor, engine_work_t *work);

typedef void (*preprocessor_finit_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor);
typedef int (*preprocessor_init_fn)(struct ey_engine *engine, struct ey_preprocessor *preprocessor);

typedef struct ey_preprocessor
{
	TAILQ_ENTRY(ey_preprocessor) link;

	char name[64];
	void *processor_data;
	
	preprocessor_init_fn preprocessor_init;		//called while preprocessor creating
	preprocessor_load_fn preprocessor_load;		//called while loading a signature
	preprocessor_load_finish_fn preprocessor_load_finish;	//called while loading all signature
	preprocessor_detect_init_fn preprocessor_detect_init;	//called while a work being created
	preprocessor_detect_fn preprocessor_detect;				//detect main
	preprocessor_detect_finit_fn preprocessor_detect_finit;	//called while a work being destroyed
	preprocessor_finit_fn preprocessor_finit;	//called while signature unload
}ey_preprocessor_t;
typedef TAILQ_HEAD(ey_preprocessor_list, ey_preprocessor) ey_preprocessor_list_t;

extern int ey_preprocessor_register(struct ey_engine *engine, ey_preprocessor_t *preprocessor);
extern int ey_preprocessor_init(struct ey_engine *engine);
extern void ey_preprocessor_finit(struct ey_engine *engine);
extern ey_preprocessor_t *ey_preprocessor_find(struct ey_engine *engine, const char *name);

#endif
