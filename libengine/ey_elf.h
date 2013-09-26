#ifndef EY_ELF_H
#define EY_ELF_H 1

#include "ey_import.h"
struct ey_engine;
typedef int (*elf_foreach)(void *data, void *start, void *arg);
extern int ey_elf_read_init(struct ey_engine *eng, void *lib_handle, const char *libname, init_handler *init, char **init_name);
extern int ey_elf_read_finit(struct ey_engine *eng, void *lib_handle, const char *libname, finit_handler *finit, char **finit_name);
extern int ey_elf_read_type(struct ey_engine *eng, const char *libname, elf_foreach cb, void *arg);
extern int ey_elf_read_ident(struct ey_engine *eng, const char *libname, elf_foreach cb, void *arg);
#endif
