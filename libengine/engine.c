#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ey_info.h"
#include "ey_memory.h"
#include "libengine.h"
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
	
	/*init event*/
	if(ey_event_init(ret))
		goto failed;

	/*init signature*/
	if(ey_signature_init(ret))
		goto failed;
	
	/*init JIT compiler*/
	if(ey_compiler_init(ret))
		goto failed;

	/*init import*/
	if(ey_import_init(ret))
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
	
	ey_signature_finit(eng);
	ey_import_finit(eng);
	ey_compiler_finit(eng);
	ey_parser_finit(eng);
	ey_event_finit(eng);
	ey_free(eng);
}

int ey_engine_load(engine_t engine, char *files[], int files_num)
{
	if(!engine || !files || files_num<=0)
	{
		engine_init_error("%s bad paramters\n", __FUNCTION__);
		return -1;
	}
	ey_engine_t *eng = (ey_engine_t *)engine;
	int index;
	for(index=0; index<files_num; index++)
	{
		if(ey_parse_file(eng, files[index]))
		{
			engine_init_error("load %s failed\n", files[index]);
			return -1;
		}
	}

	if(ey_load_post_action(eng))
	{
		engine_init_error("do post parsing action failed\n");
		return -1;
	}
	return 0;
}

engine_work_t *ey_engine_work_create(engine_t engine)
{
	/*TODO*/
	return NULL;
}

void ey_engine_work_destroy(engine_work_t *work)
{
	/*TODO*/
	return;
}

int ey_engine_work_detect(engine_work_event_t *event, engine_action_t *action)
{
	/*TODO*/
	return 0;
}

int _ey_set_event_init(engine_t engine, const char *event, int type,
	const char *function, event_init_handle address, const char *filename, int line)
{
	ey_location_t location = {line,0,line,0,filename};
	return ey_event_set_runtime_init((ey_engine_t*)engine, event, type, function, address, &location);
}

int _ey_set_event_finit(engine_t engine, const char *event, int type,
	const char *function, event_finit_handle address, const char *filename, int line)
{
	ey_location_t location = {line,0,line,0,filename};
	return ey_event_set_runtime_finit((ey_engine_t*)engine, event, type, function, address, &location);
}

int _ey_set_work_init(engine_t engine, int type, const char *function, work_init_handle address, const char *filename, int line)
{
	ey_location_t location = {line,0,line,0,filename};
	return ey_work_set_runtime_init((ey_engine_t*)engine, type, function, address, &location);
}

int _ey_set_work_finit(engine_t engine, int type, const char *function, work_finit_handle address, const char *filename, int line)
{
	ey_location_t location = {line,0,line,0,filename};
	return ey_work_set_runtime_finit((ey_engine_t*)engine, type, function, address, &location);
}

int _ey_add_file_init(engine_t engine, const char *function, file_init_handle address, const char *filename, int line)
{
	ey_location_t location = {line,0,line,0,filename};
	return ey_signature_add_init((ey_engine_t*)engine, function, address, &location);
}

int _ey_add_file_finit(engine_t engine, const char *function, file_finit_handle address, const char *filename, int line)
{
	ey_location_t location = {line,0,line,0,filename};
	return ey_signature_add_finit((ey_engine_t*)engine, function, address, &location);
}
