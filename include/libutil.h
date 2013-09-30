#ifndef LIBUTIL_H
#define LIBUTIL_H 1

extern char *ey_basename(const char *filename);
extern char *ey_fileextension(const char *filename);
extern int ey_file_is_source(const char *filename);
extern int ey_file_is_header(const char *filename);
extern int ey_file_is_library(const char *filename);
extern int ey_file_is_signature(const char *filename);
#endif
