#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "libtcc.h"
#include "libjit.h"
#include "ey_memory.h"

static char *jit_basename(const char *name)
{
    char *p = strchr(name, 0);
    while (p > name && p[-1]!='/')
        --p;
    return p;
}

static char *jit_fileextension(const char *name)
{
    char *b = jit_basename(name);
    char *e = strrchr(b, '.');
    return e ? e : strchr(b, 0);
}

ey_jit_t *ey_jit_create(const char *option, void *priv_data, error_handle error_callback)
{
	TCCState *jit = tcc_new();
	if(!jit)
	{
		fprintf(stderr, "malloc jit failed\n");
		goto failed;
	}
	
	if(option && tcc_set_options(jit, option))
	{
		fprintf(stderr, "set jit option failed\n");
		goto failed;
	}
	
	if(tcc_set_output_type(jit, TCC_OUTPUT_MEMORY))
	{
		fprintf(stderr, "set jit output type failed\n");
		goto failed;
	}

	tcc_set_private_data(jit, priv_data);
	tcc_set_error_func(jit, error_callback);
	return (ey_jit_t*)jit;

failed:
	if(jit)
		tcc_delete(jit);
	return NULL;
}

void ey_jit_destroy(ey_jit_t *jit)
{
	if(jit)
		tcc_delete((TCCState*)jit);
}

void* ey_jit_get_private_data(ey_jit_t *jit)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return NULL;
	}
	return tcc_get_private_data((TCCState*)jit);
}

int ey_jit_add_include_path(ey_jit_t *jit, const char *pathname)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!pathname)
	{
		fprintf(stderr, "null pathname\n");
		return -1;
	}

	return tcc_add_include_path((TCCState*)jit, pathname);
}

int ey_jit_add_sysinclude_path(ey_jit_t *jit, const char *pathname)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!pathname)
	{
		fprintf(stderr, "null pathname\n");
		return -1;
	}

	return tcc_add_sysinclude_path((TCCState*)jit, pathname);
}

int ey_jit_define_symbol(ey_jit_t *jit, const char *sym, const char *value)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!sym)
	{
		fprintf(stderr, "null symbol\n");
		return -1;
	}

	tcc_define_symbol((TCCState*)jit, sym, value);
	return 0;
}

int ey_jit_undefine_symbol(ey_jit_t *jit, const char *sym)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!sym)
	{
		fprintf(stderr, "null symbol\n");
		return -1;
	}

	tcc_undefine_symbol((TCCState*)jit, sym);
	return 0;
}

int ey_jit_compile_file(ey_jit_t *jit, const char *filename)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!filename)
	{
		fprintf(stderr, "null pathname\n");
		return -1;
	}

	const char *ext = jit_fileextension(filename);
	if(ext[0])
		ext++;

	if(strcmp(ext, "c"))
	{
		fprintf(stderr, "eyoung jit compiler can only compile c language source file\n");
		return -1;
	}

	return tcc_add_file((TCCState*)jit, filename);
}

int ey_jit_compile_string(ey_jit_t *jit, const char *code)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!code)
	{
		fprintf(stderr, "null code\n");
		return -1;
	}

	return tcc_compile_string((TCCState*)jit, code);
}

int ey_jit_add_library_path(ey_jit_t *jit, const char *pathname)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!pathname)
	{
		fprintf(stderr, "null pathname\n");
		return -1;
	}

	return tcc_add_library_path((TCCState*)jit, pathname);
}

int ey_jit_add_library(ey_jit_t *jit, const char *libname)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!libname)
	{
		fprintf(stderr, "null libname\n");
		return -1;
	}
	
	return tcc_add_library((TCCState*)jit, libname);
}

int ey_jit_add_symbol(ey_jit_t *jit, const char *name, const void *value)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}

	if(!name || !value)
	{
		fprintf(stderr, "null name or value\n");
		return -1;
	}
	
	return tcc_add_symbol((TCCState*)jit, name, value);
}

int ey_jit_relocate(ey_jit_t *jit, void *ptr)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return -1;
	}
	
	return tcc_relocate((TCCState*)jit, ptr);
}

void *ey_jit_get_symbol(ey_jit_t *jit, const char *name)
{
	if(!jit)
	{
		fprintf(stderr, "null jit context\n");
		return NULL;
	}

	if(!name)
	{
		fprintf(stderr, "null symbol name\n");
		return NULL;
	}
	
	return tcc_get_symbol((TCCState*)jit, name);
}
