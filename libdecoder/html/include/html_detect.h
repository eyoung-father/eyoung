#ifndef HTML_DETECT_H
#define HTML_DETECT_H 1

#include "html.h"

typedef void *xss_work_t;
typedef int (*xss_preprocess_fn)(int argc, html_string_t *argv, html_string_t **out_argv);

extern xss_work_t xss_traning(const char *template, xss_preprocess_fn fn, int argc, html_string_t *argv);
extern int xss_check(xss_work_t work, int argc, html_string_t *argv);
extern void xss_untraining(xss_work_t work);

extern int xss_module_init(void *arg);
extern int xss_module_finit(void *arg);
#endif
