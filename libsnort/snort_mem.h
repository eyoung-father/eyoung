#ifndef SNORT_MEM_H
#define SNORT_MEM_H 1

#include <stdlib.h>

#define snort_malloc(sz) malloc(sz)
#define snort_free(p) free(p)
#define snort_realloc(p,s) realloc(p,s)

#endif
