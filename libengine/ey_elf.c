#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ey_engine.h"
#include "ey_elf.h"
#include "engine_mem.h"

int ey_elf_read_init(ey_engine_t *eng, const char *libname, init_handler *init, char **init_name)
{
	return 0;
}

int ey_elf_read_finit(ey_engine_t *eng, const char *libname, finit_handler *finit, char **finit_name)
{
	return 0;
}

int ey_elf_read_type(ey_engine_t *eng, const char *libname, elf_print cb, void *arg)
{
	return 0;
}

int ey_elf_read_ident(ey_engine_t *eng, const char *libname, elf_print cb, void *arg)
{
	return 0;
}
