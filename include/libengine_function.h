#ifndef LIBENGINE_FUNCTION_H
#define LIBENGINE_FUNCTION_H 1

#include "libengine_type.h"
#include "libengine_export.h"
extern engine_t ey_engine_create(const char *name);
extern void ey_engine_destroy(engine_t engine);
extern int ey_engine_load(engine_t engine, char *files[], int files_num);

extern engine_work_t *ey_engine_work_create(engine_t engine);
extern void ey_engine_work_destroy(engine_work_t *work);
extern int ey_engine_work_detect(engine_work_event_t *event);

extern int debug_engine_parser;
extern int debug_engine_lexier;
extern int debug_engine_init;
extern int debug_engine_compiler;
extern int debug_engine_runtime;

#endif
