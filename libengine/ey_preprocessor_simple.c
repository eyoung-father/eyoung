#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "libengine.h"
#include "libmatch.h"
#include "ey_memory.h"
#include "ey_engine.h"
#include "ey_preprocessor_simple.h"

static int simple_init(ey_engine_t *engine, ey_preprocessor_t *preprocessor)
{
	assert(engine != NULL && preprocessor != NULL);

	ey_acsm_t ac = ey_acsm_create();
	if(!ac)
	{
		engine_init_error("create simple preprocessor acsm failed\n");
		return -1;
	}

	preprocessor->processor_data = ac;
	return 0;
}

static void simple_finit(ey_engine_t *engine, ey_preprocessor_t *preprocessor)
{
	assert(engine != NULL && preprocessor != NULL);
	if(preprocessor->processor_data)
	{
		ey_acsm_t ac = (ey_acsm_t)preprocessor->processor_data;
		ey_acsm_destroy(ac);
	}
}

static int simple_load_finish(ey_engine_t *engine, ey_preprocessor_t *preprocessor)
{
	assert(engine != NULL && preprocessor != NULL);
	if(preprocessor->processor_data)
	{
		ey_acsm_t ac = (ey_acsm_t)preprocessor->processor_data;
		if(ey_acsm_compile(ac))
		{
			engine_compiler_error("compile acsm failed\n");
			return -1;
		}
	}
	return 0;
}

static int simple_load(ey_engine_t *engine, ey_preprocessor_t *preprocessor, const char *signature, unsigned long id)
{
	assert(engine != NULL && preprocessor != NULL && signature != NULL);
	
	if(preprocessor->processor_data)
	{
		ey_acsm_t ac = (ey_acsm_t)preprocessor->processor_data;
		ey_acsm_pattern_t pattern;
		memset(&pattern, 0, sizeof(ey_acsm_pattern_t));

		pattern.pattern = (char*)signature;
		pattern.pattern_len = strlen(signature);
		pattern.id = (void*)id;
		pattern.nocase = 1;
		if(ey_acsm_add_pattern(ac, &pattern))
		{
			engine_parser_error("load signature %llu:%s failed\n", id, signature);
			return -1;
		}
	}
	return 0;
}

static int simple_detect_init(ey_engine_t *engine, ey_preprocessor_t *preprocessor, engine_work_t *work)
{
	assert(engine != NULL && preprocessor != NULL && work != NULL && work->priv_data != NULL);

	ey_work_t *w = (ey_work_t*)work->priv_data;
	ey_simple_private_t *data = NULL;

	data = (ey_simple_private_t*)engine_malloc(sizeof(ey_simple_private_t));
	if(!data)
	{
		engine_runtime_error("malloc simple preprocessor data failed\n");
		return -1;
	}
	memset(data, 0, sizeof(ey_simple_private_t));
	w->simple_preprocessor = data;
	return 0;
}

static void simple_detect_finit(ey_engine_t *engine, ey_preprocessor_t *preprocessor, engine_work_t *work)
{
	assert(engine != NULL && preprocessor != NULL && work != NULL && work->priv_data != NULL);

	ey_work_t *w = (ey_work_t*)work->priv_data;
	ey_simple_private_t *data = w->simple_preprocessor;

	if(data)
		engine_free(data);
	w->simple_preprocessor = NULL;
}

static int simple_detect(ey_engine_t *engine, ey_preprocessor_t *preprocessor, engine_work_t *work, 
	const char *buf, size_t buf_len, int from_client)
{
	return 0;
}

int ey_preprocessor_simple_register(ey_engine_t *engine)
{
	assert(engine != NULL);

	ey_preprocessor_t pp;
	memset(&pp, 0, sizeof(pp));

	strncpy(pp.name, "simple-preprocessor", sizeof(pp.name));
	pp.preprocessor_init = simple_init;
	pp.preprocessor_load = simple_load;
	pp.preprocessor_load_finish = simple_load_finish;
	pp.preprocessor_detect_init = simple_detect_init;
	pp.preprocessor_detect_finit = simple_detect_finit;
	pp.preprocessor_detect = simple_detect;
	pp.preprocessor_finit = simple_finit;

	return ey_preprocessor_register(engine, &pp);
}
