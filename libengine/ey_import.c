#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ey_engine.h"
#include "ey_import.h"
#include "ey_export.h"

static unsigned long hash_filename(void *filename)
{
	return ((unsigned long)filename)>>3;
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

static int destroy_each(void *l, void *e)
{
	ey_library_t *lib = (ey_library_t*)l;
	ey_engine_t *eng = (ey_engine_t*)e;
	finit_handler finit_func = lib->finit;
	if(finit_func)
		engine_parser_debug("finit function for %s returns %d\n", lib->name, (*finit_func)(eng));
	else
		engine_parser_debug("no finit function for %s\n", lib->name?lib->name:"(NULL)");

	if(lib->finit_name)
		engine_fzfree(ey_parser_fslab(eng), lib->finit_name);
	if(lib->init_name)
		engine_fzfree(ey_parser_fslab(eng), lib->init_name);
	if(lib->lib_handle)
		dlclose(lib->lib_handle);
	engine_fzfree(ey_parser_fslab(eng), lib);
	return 0;
}

void ey_import_finit(ey_engine_t *eng)
{
	if(ey_library_hash(eng))
	{
		ey_hash_foreach(ey_library_hash(eng), NULL, NULL, destroy_each, (void*)eng);
		ey_hash_destroy(ey_library_hash(eng));
		ey_library_hash(eng) = NULL;
	}
}

int ey_attach_library(struct ey_engine *eng, char *libname)
{
	init_handler init_func = NULL;
	char *init_name = NULL;

	finit_handler finit_func = NULL;
	char *finit_name = NULL;

	void *lib_handle = NULL;

	ey_library_t *lib = NULL;

	if(!eng || !libname)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		goto failed;
	}

	if(ey_hash_find(ey_library_hash(eng), (void*)libname))
	{
		engine_parser_debug("%s is already imported\n", libname);
		return 0;
	}
	
	lib_handle = dlopen(libname, RTLD_NOW|RTLD_GLOBAL);
	if(!lib_handle)
	{
		engine_parser_error("open library failed: %s\n", dlerror());
		goto failed;
	}

	if(ey_elf_read_init(eng, lib_handle, libname, &init_func, &init_name))
	{
		engine_parser_error("parse %s init section failed\n", libname);
		goto failed;
	}

	if(ey_elf_read_finit(eng, lib_handle, libname, &finit_func, &finit_name))
	{
		engine_parser_error("parse %s finit section failed\n", libname);
		goto failed;
	}

	lib = (ey_library_t*)engine_fzalloc(sizeof(ey_library_t), ey_parser_fslab(eng));
	if(!lib)
	{
		engine_parser_error("malloc ey_library_t failed\n");
		goto failed;
	}
	memset(lib, 0, sizeof(*lib));
	lib->name = libname;
	lib->lib_handle = lib_handle;
	lib->init_name = init_name;
	lib->init = init_func;
	lib->finit_name = finit_name;
	lib->finit = finit_func;
	
	if(ey_hash_insert(ey_library_hash(eng), (void*)libname, (void*)lib))
	{
		engine_parser_error("insert hash failed\n");
		goto failed;
	}
	
	if(init_func)
		engine_parser_debug("init function for %s returns %d\n", libname, (*init_func)(eng));
	else
		engine_parser_debug("no init function\n");
	return 0;

failed:
	if(lib_handle)
		dlclose(lib_handle);
	if(init_name)
		engine_fzfree(ey_parser_fslab(eng), init_name);
	if(finit_name)
		engine_fzfree(ey_parser_fslab(eng), finit_name);
	if(lib)
		engine_fzfree(ey_parser_fslab(eng), lib);
	return -1;
}

int ey_detach_library(ey_engine_t *eng, char *libname)
{
	ey_library_t *lib = NULL;
	init_handler finit_func = NULL;

	if(!eng || !libname)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}
	
	if(ey_hash_remove(ey_library_hash(eng), (void*)libname, (void**)&lib))
	{
		engine_parser_debug("lib %s is not attached\n", libname);
		return 0;
	}
	
	finit_func = lib->finit;
	if(finit_func)
		engine_parser_debug("finit function for %s returns %d\n", libname, (*finit_func)(eng));
	else
		engine_parser_debug("no finit function\n");

	if(lib->finit_name)
		engine_fzfree(ey_parser_fslab(eng), lib->finit_name);
	if(lib->init_name)
		engine_fzfree(ey_parser_fslab(eng), lib->init_name);
	if(lib->lib_handle)
		dlclose(lib->lib_handle);
	engine_fzfree(ey_parser_fslab(eng), lib);
	return 0;
}

static int library_output(void *data, void *start, void *fp)
{
	ey_extern_symbol_t *symbol = (ey_extern_symbol_t*)data;
	if(symbol->decl)
		fprintf((FILE*)fp, "%s\n", (char*)(start + (unsigned long)symbol->decl));
	return 0;
}

int ey_extract_library(ey_engine_t *eng, char *libname, FILE *out_fp)
{
	ey_library_t *lib = NULL;

	if(!eng || !libname || !out_fp)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}
	
	lib = ey_hash_find(ey_library_hash(eng), (void*)libname);
	if(!lib)
	{
		engine_parser_error("lib %s is not attached\n", libname);
		return -1;
	}

	if(ey_elf_read_type(eng, libname, library_output, out_fp))
	{
		engine_parser_error("read type section in library %s failed\n", libname);
		return -1;
	}

	if(ey_elf_read_ident(eng, libname, library_output, out_fp))
	{
		engine_parser_error("read ident section in library %s failed\n", libname);
		return -1;
	}

	return 0;
}
