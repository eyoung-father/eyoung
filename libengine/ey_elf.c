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

static int ey_elf_read(ey_engine_t *eng, const char *libname, const char *section, elf_foreach cb, void *arg)
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
	size_t n, shstrndx, index=0;
	
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
			for(n=0, p=(ey_extern_symbol_t*)data->d_buf; n<shdr.sh_size/sizeof(ey_extern_symbol_t); n++, p++)
			{
				engine_parser_debug("symbol[%d]: %s\n", n, p->name);
				if(cb((void*)p, arg))
				{
					engine_parser_debug("elf foreach function returns non-zero\n");
					break;
				}
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
	void *func;
	char *name;
}read_arg_t;
static int check_one;

static int read_nit(void *d, void *a)
{
	ey_extern_symbol_t *symbol = (ey_extern_symbol_t*)d;
	read_arg_t *arg = (read_arg_t*)a;

	if(check_one)
	{
		engine_parser_error("more than one function\n");
		return 1;
	}
	check_one++;
	arg->func = symbol->value;
	arg->name = symbol->name;
	return 0;
}

int ey_elf_read_init(ey_engine_t *eng, const char *libname, init_handler *init, char **init_name)
{
	if(!eng || !libname || !init || !init_name)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	read_arg_t arg = {NULL, NULL};
	*init_name = NULL;
	*init = NULL;
	
	if(ey_elf_read(eng, libname, EY_INIT_SECTION, read_nit, (void*)&arg))
	{
		engine_parser_error("read library %s init section failed\n");
		return -1;
	}

	*init = (init_handler)(arg.func);
	if(arg.name)
	{
		*init_name = (char*)engine_fzalloc(strlen(arg.name)+1, ey_parser_fslab(eng));
		if(!*init_name)
		{
			engine_parser_error("alloc init function name failed\n");
			return -1;
		}
		strcpy(*init_name, arg.name);
	}
	return 0;
}

int ey_elf_read_finit(ey_engine_t *eng, const char *libname, finit_handler *finit, char **finit_name)
{
	if(!eng || !libname || !finit || !finit_name)
	{
		engine_parser_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	read_arg_t arg = {NULL, NULL};
	*finit_name = NULL;
	*finit = NULL;
	
	if(ey_elf_read(eng, libname, EY_FINIT_SECTION, read_nit, (void*)&arg))
	{
		engine_parser_error("read library %s finit section failed\n");
		return -1;
	}

	*finit = (finit_handler)(arg.func);
	if(arg.name)
	{
		*finit_name = (char*)engine_fzalloc(strlen(arg.name)+1, ey_parser_fslab(eng));
		if(!*finit_name)
		{
			engine_parser_error("alloc finit function name failed\n");
			return -1;
		}
		strcpy(*finit_name, arg.name);
	}
	return 0;
}

int ey_elf_read_type(ey_engine_t *eng, const char *libname, elf_foreach cb, void *arg)
{
	return ey_elf_read(eng, libname, EY_TYPE_SECTION, cb, arg);
}

int ey_elf_read_ident(ey_engine_t *eng, const char *libname, elf_foreach cb, void *arg)
{
	return ey_elf_read(eng, libname, EY_IDENT_SECTION, cb, arg);
}
