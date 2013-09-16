#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ey_engine.h"
#include "ey_import.h"

static unsigned int hash_filename(void *filename)
{
	return ((unsigned int)filename)>>3;
}

static int compare_filename(void *k, void *v)
{
	if(!k || !v)
		return 1;
	
	return strcmp((char*)k, ((ey_library_t*)v)->name);
}

int ey_import_init(ey_engine_t *eng)
{
	char name[64];

	if(!ey_library_hash(eng))
	{
		snprintf(name, sizeof(name), "%s library hash\n", eng->name);
		name[63] = '\0';
		ey_library_hash(eng) = ey_hash_create(name, 10, 8192, hash_filename, compare_filename, NULL, NULL);
		if(!ey_library_hash(eng))
		{
			engine_init_error("create library hash failed\n");
			return -1;
		}
	}

	return 0;
}

void ey_import_finit(ey_engine_t *eng)
{
	if(ey_library_hash(eng))
	{
		ey_hash_destroy(ey_library_hash(eng));
		ey_library_hash(eng) = NULL;
	}
}

int ey_attach_library(struct ey_engine *eng, char *libname)
{
	if(!eng)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	if(ey_hash_find(ey_library_hash(eng), (void*)libname))
	{
		engine_parser_debug("%s is already imported\n", libname);
		return 0;
	}
	return 0;
}
