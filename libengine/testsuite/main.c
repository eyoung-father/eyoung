#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libengine.h"

int main(int argc, char *argv[])
{
	engine_t engine = NULL;

	debug_engine_parser = 1;
	debug_engine_lexier = 1;
	debug_engine_init = 1;

	if(argc < 2)
	{
		engine_init_error("use filename as parameter\n");
		return -1;
	}

	engine = ey_engine_create("demo");
	if(!engine)
	{
		engine_init_error("create engine failed\n");
		return -1;
	}

	if(ey_engine_load(engine, &argv[1], argc-1))
		engine_init_error("load signature failed\n");
	
	ey_engine_destroy(engine);
	return 0;
}
