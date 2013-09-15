#ifndef EY_COMPILER_H
#define EY_COMPILER_H 1

#define SYSTEM_INCLUDE_PATH		"/usr/include/"
#define SYSTEM_LIBRARY_PATH		"/usr/lib/"

#define LOCAL_INCLUDE_PATH		"./include/"
#define LOCAL_LIBRARY_PATH		"./lib/"

struct ey_engine;
extern int ey_compiler_init(struct ey_engine *eng);
extern void ey_compiler_finit(struct ey_engine *eng);
#endif
