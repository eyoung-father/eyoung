#ifndef LIBJIT_H
#define LIBJIT_H 1

typedef void* ey_jit_t;

/*
 * common function
 * */
/* create a new JIT compiler context */
typedef void (*error_handle)(void *opaque, const char *msg);
extern ey_jit_t ey_jit_create(const char *option, void *priv_data, error_handle error_callback);

/*free JIT compiler context*/
extern void ey_jit_destroy(ey_jit_t jit);

/*
 * for preprocessor
 * */
/* add include path */
extern int ey_jit_add_include_path(ey_jit_t jit, const char *pathname);

/* add in system include path */
extern int ey_jit_add_sysinclude_path(ey_jit_t jit, const char *pathname);

/* define preprocessor symbol 'sym'. Can put optional value */
extern int ey_jit_define_symbol(ey_jit_t jit, const char *sym, const char *value);

/* undefine preprocess symbol 'sym' */
extern int ey_jit_undefine_symbol(ey_jit_t jit, const char *sym);

/*
 * for compiling
 * */
/*compile a source file in memory*/
extern int ey_jit_compile_file(ey_jit_t jit, const char *filename);

/*compile a source code string in memory*/
extern int ey_jit_compile_string(ey_jit_t jit, const char *code);

/*
 * for linkage
 * */
/* equivalent to -Lpath option */
extern int ey_jit_add_library_path(ey_jit_t jit, const char *pathname);

/* the library name is the same as the argument of the '-l' option */
extern int ey_jit_add_library(ey_jit_t jit, const char *libname);

/* add a symbol to the compiled program */
extern int ey_jit_add_symbol(ey_jit_t jit, const char *name, const void *value);

/* do all relocations */
extern int ey_jit_relocate(ey_jit_t jit, void *ptr);
/* possible values for 'ptr':
   - EY_JIT_RELOCATE_AUTO : Allocate and manage memory internally
   - NULL                 : return required memory size for the step below
   - memory address       : copy code to memory passed by the caller
   returns -1 if error. */
#define EY_JIT_RELOCATE_AUTO (void*)1

/* return symbol value or NULL if not found */
extern void *ey_jit_get_symbol(ey_jit_t jit, const char *name);
#endif
