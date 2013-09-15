#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ey_engine.h"
#include "ey_compiler.h"

int ey_compiler_init(struct ey_engine *eng)
{
	ey_jit_t jit = ey_jit_create(NULL, NULL, NULL);
	if(!jit)
	{
		engine_init_error("create JIT compiler failed\n");
		goto failed;
	}

	if(ey_jit_add_sysinclude_path(jit, SYSTEM_INCLUDE_PATH))
	{
		engine_init_error("add system include path failed\n");
		goto failed;
	}

	if(ey_jit_add_include_path(jit, LOCAL_INCLUDE_PATH))
	{
		engine_init_error("add local include path failed\n");
		goto failed;
	}

	if(ey_jit_add_library_path(jit, SYSTEM_LIBRARY_PATH))
	{
		engine_init_error("add system library path failed\n");
		goto failed;
	}

	if(ey_jit_add_library_path(jit, LOCAL_LIBRARY_PATH))
	{
		engine_init_error("add local library path failed\n");
		goto failed;
	}

	ey_jit(eng) = jit;
	return 0;

failed:
	if(jit)
		ey_jit_destroy(jit);

	return -1;
}

void ey_compiler_finit(struct ey_engine *eng)
{
	if(!eng)
		return;

	if(ey_jit(eng))
		ey_jit_destroy(eng->jit);
	ey_jit(eng) = NULL;
}
