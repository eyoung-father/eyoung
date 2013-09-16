#ifndef EY_IMPORT_H
#define EY_IMPORT_H 1

#include <stdio.h>
#include "ey_queue.h"
struct ey_engine;

typedef int (*init_handler)(struct ey_engine *eng);
typedef int (*finit_handler)(struct ey_engine *eng);
typedef struct ey_library
{
	char *name;
	void *lib_handle;

	char *init_name;
	init_handler init;

	char *finit_name;
	finit_handler finit;
}ey_library_t;

extern int ey_import_init(struct ey_engine *eng);
extern void ey_import_finit(struct ey_engine *eng);
extern int ey_attach_library(struct ey_engine *eng, char *libname);
extern int ey_detach_library(struct ey_engine *eng, char *libname);
extern int ey_extract_library(struct ey_engine *eng, char *libname, FILE *out_fp);
#endif
