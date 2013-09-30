#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "libutil.h"

char *ey_basename(const char *filename)
{
	char *p = strchr(filename, 0);
	while (p > filename && p[-1]!='/')
		--p;
	return p;
}

char *ey_fileextension(const char *filename)
{
	char *b = ey_basename(filename);
	char *e = strrchr(b, '.');
	return e ? e : strchr(b, 0);
}

int ey_file_is_source(const char *filename)
{
	const char *ext = ey_fileextension(filename);
	if(ext[0])
		ext++;

	return !strcmp(ext, "c") || !strcmp(ext, "eyc");
}

int ey_file_is_header(const char *filename)
{
	const char *ext = ey_fileextension(filename);
	if(ext[0])
		ext++;

	return !strcmp(ext, "h");
}

int ey_file_is_library(const char *filename)
{
	const char *ext = ey_fileextension(filename);
	if(ext[0])
		ext++;

	return !strcmp(ext, "so");
}

int ey_file_is_signature(const char *filename)
{
	const char *ext = ey_fileextension(filename);
	if(ext[0])
		ext++;

	return !strcmp(ext, "ey");
}
