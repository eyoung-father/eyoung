/*
*  * Print the names of ELF sections.
*   */

#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sysexits.h>
#include <unistd.h>

#include "ey_export.h"

EY_EXPORT_TYPE(hill, "struct s{int a; int b;};");
EY_EXPORT_TYPE(stone, "struct t{int x; int y;};");

int
main(int argc, char **argv)
{
	int fd, flag;
	Elf *e;
	char *name, *p, *sym_start;
	Elf_Scn *scn, *rodata_scn;
	Elf_Data *data, *rodata_data;
	GElf_Shdr shdr, rodata_shdr;
	size_t n, shstrndx, sz, ey_type_index=0, rodata_index=0;

	if (argc != 3)
		errx(EX_USAGE, "usage: %s file-name section_name", argv[0]);

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EX_SOFTWARE, "ELF library initialization failed: %s", elf_errmsg(-1));

	if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
		err(EX_NOINPUT, "open \%s\" failed", argv[1]);

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		errx(EX_SOFTWARE, "elf_begin() failed: %s.", elf_errmsg(-1));

	if (elf_kind(e) != ELF_K_ELF)
		errx(EX_DATAERR, "%s is not an ELF object.", argv[1]);

	if (elf_getshstrndx(e, &shstrndx) == 0) 
		errx(EX_SOFTWARE, "getshstrndx() failed: %s.", elf_errmsg(-1));

	scn = NULL; 
	while ((scn = elf_nextscn(e, scn)) != NULL) 
	{ 
		if (gelf_getshdr(scn, &shdr) != &shdr) 
			errx(EX_SOFTWARE, "getshdr() failed: %s.", elf_errmsg(-1));

		if ((name = elf_strptr(e, shstrndx, shdr.sh_name)) == NULL) 
			errx(EX_SOFTWARE, "elf_strptr() failed: %s.", elf_errmsg(-1));

		if(!strcmp(name, argv[2]))
			ey_type_index = elf_ndxscn(scn);

		if(!strcmp(name, ".rodata"))
			rodata_index = elf_ndxscn(scn);
	}
	
	if(ey_type_index)
	{
		if ((scn = elf_getscn(e, ey_type_index)) == NULL)         
			errx(EX_SOFTWARE, "getscn() failed: %s.", elf_errmsg(-1));

		if (gelf_getshdr(scn, &shdr) != &shdr)
			errx(EX_SOFTWARE, "getshdr(shstrndx) failed: %s.", elf_errmsg(-1));

		if ((rodata_scn = elf_getscn(e, rodata_index)) == NULL)         
			errx(EX_SOFTWARE, "getscn() failed: %s.", elf_errmsg(-1));

		if (gelf_getshdr(rodata_scn, &rodata_shdr) != &rodata_shdr)
			errx(EX_SOFTWARE, "getshdr(shstrndx) failed: %s.", elf_errmsg(-1));
		
		rodata_data = NULL;
		rodata_data = elf_getdata(rodata_scn, rodata_data);
		if(!rodata_data)
			errx(EX_SOFTWARE, "elf_getdata(rodata_scn) failed: %s.", elf_errmsg(-1));

		data = NULL; 
		n = 0;
		ey_extern_symbol_t *p=NULL;
		data = elf_getdata(scn, data);
		if(data)
		{
			for(n=0, p=(ey_extern_symbol_t*)data->d_buf; n<shdr.sh_size/sizeof(ey_extern_symbol_t); n++, p++)
				printf("type[%d]: %s, %s, %s\n", n, 
					p->name?(char*)(rodata_data->d_buf + (unsigned long)p->name - rodata_shdr.sh_addr):"null",
					p->decl?(char*)(rodata_data->d_buf + (unsigned long)p->decl - rodata_shdr.sh_addr):"null",
					p->file?(char*)(rodata_data->d_buf + (unsigned long)p->file - rodata_shdr.sh_addr):"null");
		}
	}

	(void) elf_end(e);
	(void) close(fd);
	exit(EX_OK);
}
