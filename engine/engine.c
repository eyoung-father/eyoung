#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ey_info.h"
#include "ey_memory.h"
#include "engine.h"
#include "ey_engine.h"

engine_t ey_engine_create(const char *name)
{
	ey_engine_t *ret = NULL;
	ret = ey_malloc(sizeof(*ret));
	if(!ret)
	{
		engine_init_error("create engine failed\n");
		goto failed;
	}
	memset(ret, 0, sizeof(*ret));

	/*init engine name*/
	snprintf(ret->name, sizeof(ret->name), name);
	ret->name[sizeof(ret->name)-1] = '\0';
	
	/*init parser*/
	if(ey_parser_init(ret))
		goto failed;
	
	return (engine_t)ret;

failed:
	ey_engine_destroy(ret);
	return NULL;
}

void ey_engine_destroy(engine_t engine)
{
	ey_engine_t *eng = (ey_engine_t *)engine;
	if(!eng)
		return;
	
	ey_parser_finit(eng);
	ey_free(eng);
}

int ey_engine_load(engine_t engine, const char *root_file)
{
	ey_engine_t *eng = (ey_engine_t *)engine;
	return ey_parse_file(eng, root_file);
}

engine_work_t ey_engine_work_create(engine_t engine)
{
	/*TODO*/
	return 0;
}

void ey_engine_work_destroy(engine_t engine, engine_work_t work)
{
	/*TODO*/
	return;
}

int ey_engine_work_detect(engine_t eng, engine_work_t work, engine_event_t *event, engine_action_t *action)
{
	/*TODO*/
	return 0;
}


#ifdef ENGINE_MAIN
int main(int argc, char *argv[])
{
	engine_t engine = NULL;

	gram_debug = 1;
	debug_engine_parser = 1;
	debug_engine_lexier = 1;
	debug_engine_init = 1;

	if(argc != 2)
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

	if(ey_engine_load(engine, argv[1]))
		engine_init_error("load signature failed\n");
	
	ey_engine_destroy(engine);
	return 0;
}
#endif
