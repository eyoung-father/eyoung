#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <gelf.h>
#include <unistd.h>
#include <fcntl.h>

#include "ey_engine.h"
#include "ey_elf.h"
#include "engine_mem.h"
#include "ey_export.h"

static int ey_elf_read(ey_engine_t *eng, void *lib_handle, const char *libname, const char *section, elf_foreach cb, void *arg)
{
	if(!eng || !libname || !section || !cb)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	int fd = -1;
	Elf *e = NULL;
	char *name = NULL;
	Elf_Scn *scn = NULL;
	Elf_Data *data = NULL;
	GElf_Shdr shdr;
	Elf_Scn *rodata_scn = NULL;
	Elf_Data *rodata_data = NULL;
	GElf_Shdr rodata_shdr;
	size_t n, shstrndx, index=0, rodata_index=0, sz=0;
	
	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		engine_parser_error("ELF library initialization failed: %s", elf_errmsg(-1));
		goto failed;
	}

	if ((fd = open(libname, O_RDONLY, 0)) < 0)
	{
		engine_parser_error("open \%s\" failed", libname);
		goto failed;
	}

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
	{
		engine_parser_error("elf_begin() failed: %s.", elf_errmsg(-1));
		goto failed;
	}

	if (elf_kind(e) != ELF_K_ELF)
	{
		engine_parser_error("%s is not an ELF object.", libname);
		goto failed;
	}

	if (elf_getshstrndx(e, &shstrndx) == 0) 
	{
		engine_parser_error("getshstrndx() failed: %s.", elf_errmsg(-1));
		goto failed;
	}

	scn = NULL; 
	while ((scn = elf_nextscn(e, scn)) != NULL) 
	{ 
		if (gelf_getshdr(scn, &shdr) != &shdr) 
		{
			engine_parser_error("getshdr() failed: %s.", elf_errmsg(-1));
			goto failed;
		}

		if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL) 
		{
			engine_parser_error("elf_strptr() failed: %s.", elf_errmsg(-1));
			goto failed;
		}

		if(!strcmp(name, section))
			index = elf_ndxscn(scn);
		
		if(!strcmp(name, ".rodata"))
			rodata_index = elf_ndxscn(scn);
	}
	
	if(!rodata_index)
	{
		engine_parser_error("get .rodata section failed\n");
		goto failed;
	}
	
	if ((rodata_scn = elf_getscn(e, rodata_index)) == NULL)         
	{
		engine_parser_error("getscn() for rodata failed: %s.", elf_errmsg(-1));
		goto failed;
	}

	if (gelf_getshdr(rodata_scn, &rodata_shdr) != &rodata_shdr)
	{
		engine_parser_error("rodata getshdr(shstrndx) failed: %s.", elf_errmsg(-1));
		goto failed;
	}
	rodata_data = elf_getdata(rodata_scn, rodata_data);
	if(!rodata_data)
	{
		engine_parser_error("rodata getdata failed: %s.", elf_errmsg(-1));
		goto failed;
	}

	if(index)
	{
		if ((scn = elf_getscn(e, index)) == NULL)         
		{
			engine_parser_error("getscn() failed: %s.", elf_errmsg(-1));
			goto failed;
		}

		if (gelf_getshdr(scn, &shdr) != &shdr)
		{
			engine_parser_error("getshdr(shstrndx) failed: %s.", elf_errmsg(-1));
			goto failed;
		}

		data = NULL; 
		n = 0;
		ey_extern_symbol_t *p=NULL;
		data = elf_getdata(scn, data);
		if(data)
		{
			int elm_size = sizeof(ey_extern_symbol_t);
			void *start = rodata_data->d_buf - rodata_shdr.sh_addr;
			if(shdr.sh_addralign && (elm_size & (shdr.sh_addralign-1)))
				elm_size = (elm_size & ~(shdr.sh_addralign-1)) + shdr.sh_addralign;
			engine_parser_debug("get elm size: %d\n", elm_size);

			n = 0;
			sz = 0;
			p=(ey_extern_symbol_t*)data->d_buf;
			while(sz < shdr.sh_size)
			{
				engine_parser_debug("symbol[%d]:\n", n);
				engine_parser_debug("  name: %s\n", p->name?(char*)(start + (unsigned long)p->name):"null");
				engine_parser_debug("  decl: %s\n", p->decl?(char*)(start + (unsigned long)p->decl):"null");
				engine_parser_debug("  pos: %s:%d\n", p->file?(char*)(start + (unsigned long)p->file):"null", p->line);
				engine_parser_debug("  value: %p\n", p->value);
				if(cb((void*)p, start, arg))
				{
					engine_parser_debug("elf foreach function returns non-zero\n");
					break;
				}
				p = (ey_extern_symbol_t*)((void*)p + elm_size);
				sz += elm_size;
				n++;
			}
		}
	}

	elf_end(e);
	close(fd);
	return 0;

failed:
	if(e)
		elf_end(e);
	if(fd>=0)
		close(fd);
	return -1;
}

typedef struct read_arg
{
	char *name;
	ey_engine_t *engine;
}read_arg_t;
static int check_one;

static int read_nit(void *d, void *start, void *a)
{
	ey_extern_symbol_t *symbol = (ey_extern_symbol_t*)d;
	read_arg_t *arg = (read_arg_t*)a;

	if(check_one)
	{
		engine_parser_error("more than one function\n");
		return 1;
	}
	check_one++;
	char *name = (char*)(start + (unsigned long)symbol->name);
	arg->name = (char*)engine_fzalloc(strlen(name) + 1, ey_parser_fslab(arg->engine));
	if(!arg->name)
	{
		engine_parser_error("alloc function name %s failed\n", name);
		return 1;
	}
	strcpy(arg->name, name);
	return 0;
}

int ey_elf_read_init(ey_engine_t *eng, void *lib_handle, const char *libname, init_handler *init, char **init_name)
{
	if(!eng || !libname || !init || !init_name || !lib_handle)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	read_arg_t arg = {NULL, eng};
	*init_name = NULL;
	*init = NULL;
	check_one = 0;
	
	if(ey_elf_read(eng, lib_handle, libname, EY_INIT_SECTION, read_nit, (void*)&arg))
	{
		engine_parser_error("read library %s init section failed\n");
		return -1;
	}

	*init_name = arg.name;
	if(!*init_name)
	{
		engine_parser_debug("cannot find init function\n");
		return 0;
	}

	*init = (init_handler)dlsym(lib_handle, arg.name);
	if(!*init)
	{
		engine_parser_error("find init function %s failed\n", arg.name);
		return -1;
	}
	return 0;
}

int ey_elf_read_finit(ey_engine_t *eng, void *lib_handle, const char *libname, finit_handler *finit, char **finit_name)
{
	if(!eng || !libname || !finit || !finit_name || !lib_handle)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	read_arg_t arg = {NULL, eng};
	*finit_name = NULL;
	*finit = NULL;
	check_one = 0;
	
	if(ey_elf_read(eng, lib_handle, libname, EY_FINIT_SECTION, read_nit, (void*)&arg))
	{
		engine_parser_error("read library %s finit section failed\n");
		return -1;
	}

	*finit_name = arg.name;
	if(!*finit_name)
	{
		engine_parser_debug("cannot find finit function\n");
		return 0;
	}

	*finit = (finit_handler)dlsym(lib_handle, arg.name);
	if(!*finit)
	{
		engine_parser_error("find finit function %s failed\n", arg.name);
		return -1;
	}
	return 0;
}

int ey_elf_read_type(ey_engine_t *eng, const char *libname, elf_foreach cb, void *arg)
{
	return ey_elf_read(eng, NULL, libname, EY_TYPE_SECTION, cb, arg);
}

int ey_elf_read_ident(ey_engine_t *eng, const char *libname, elf_foreach cb, void *arg)
{
	return ey_elf_read(eng, NULL, libname, EY_IDENT_SECTION, cb, arg);
}
