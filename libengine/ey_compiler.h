#ifndef EY_COMPILER_H
#define EY_COMPILER_H 1

#define SYSTEM_INCLUDE_PATH		"/usr/include/"
#define SYSTEM_LIBRARY_PATH		"/usr/lib/"

#define LOCAL_INCLUDE_PATH		"./include/"
#define LOCAL_LIBRARY_PATH		"./lib/"

struct ey_engine;
struct ey_signature_file;
extern int ey_compiler_init(struct ey_engine *eng);
extern void ey_compiler_finit(struct ey_engine *eng);
extern int ey_compile_string(struct ey_engine *eng, const char *filename);
extern int ey_compile_signature_file(struct ey_engine *eng, struct ey_signature_file *signature_file);
extern int ey_compile_post_action(struct ey_engine *eng, struct ey_signature_file *signature_file);
extern int ey_compiler_link(struct ey_engine *eng);
#endif
