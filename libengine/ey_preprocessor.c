#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ey_engine.h"
#include "ey_preprocessor_simple.h"

static ey_preprocessor_t *preprocessor_find(ey_engine_t *engine, const char *name)
{
	ey_assert(engine != NULL && name != NULL);
	
	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &ey_preprocessor_list(engine), link)
	{
		if(!strcmp(pp->name, name))
			break;
	}
	return pp;
}

int ey_preprocessor_register(ey_engine_t *engine, ey_preprocessor_t *preprocessor)
{
	ey_preprocessor_t *ret = NULL;
	ey_assert(engine != NULL && preprocessor != NULL);
	
	if(preprocessor_find(engine, preprocessor->name))
	{
		engine_init_error("preprocessor %s is already registered\n");
		return -1;
	}

	ret = (ey_preprocessor_t*)engine_malloc(sizeof(ey_preprocessor_t));
	if(!ret)
	{
		engine_init_error("malloc preprocessor failed\n");
		return -1;
	}

	memset(ret, 0, sizeof(*ret));
	memcpy(ret, preprocessor, sizeof(ey_preprocessor_t));

	if(preprocessor->preprocessor_init && preprocessor->preprocessor_init(engine, ret))
	{
		engine_init_error("preprocessor init failed\n");
		engine_free(ret);
		return -1;
	}

	TAILQ_INSERT_TAIL(&ey_preprocessor_list(engine), ret, link);
	return 0;
}

static void ey_preprocessor_unregister(ey_engine_t *engine, ey_preprocessor_t *preprocessor)
{
	ey_assert(engine != NULL && preprocessor != NULL);

	if(preprocessor->preprocessor_finit)
		preprocessor->preprocessor_finit(engine, preprocessor);
	
	engine_free(preprocessor);
}

void ey_preprocessor_finit(ey_engine_t *engine)
{
	ey_assert(engine != NULL);

	ey_preprocessor_t *pp = NULL, *tmp = NULL;
	TAILQ_FOREACH_SAFE(pp, &ey_preprocessor_list(engine), link, tmp)
	{
		if(pp->preprocessor_finit)
			pp->preprocessor_finit(engine, pp);
		ey_preprocessor_unregister(engine, pp);
	}
	
	TAILQ_INIT(&ey_preprocessor_list(engine));
}

int ey_preprocessor_init(ey_engine_t *engine)
{
	ey_assert(engine != NULL);

	TAILQ_INIT(&ey_preprocessor_list(engine));

	/*register preprocessor*/
	if(ey_preprocessor_simple_register(engine))
	{
		engine_init_error("register simple preprocessor failed\n");
		return -1;
	}

	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &engine->preprocessor_list, link)
	{
		if(pp->preprocessor_init && pp->preprocessor_init(engine, pp))
		{
			engine_init_error("call init function for preprocessor %s failed\n", pp->name);
			return -1;
		}
		continue;
	}
	return 0;
}

int ey_preprocessor_compile(ey_engine_t *engine)
{
	ey_assert(engine != NULL);
	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &engine->preprocessor_list, link)
	{
		if(pp->preprocessor_load_finish && pp->preprocessor_load_finish(engine, pp))
		{
			engine_compiler_error("compile preprocessor %s failed\n", pp->name);
			return -1;
		}
		continue;
	}
	return 0;
}

int ey_preprocessor_load(ey_engine_t *engine, const char *pp_name, const char *signature, void *id)
{
	ey_assert(engine != NULL && pp_name != NULL && signature != NULL);

	ey_preprocessor_t *pp = preprocessor_find(engine, pp_name);
	if(!pp)
	{
		engine_parser_error("preprocessor %s is not registered\n", pp_name);
		return -1;
	}

	if(pp->preprocessor_load && pp->preprocessor_load(engine, pp, signature, (unsigned long)id))
	{
		engine_parser_error("preprocessor %s loads signature %s failed\n", pp->name, signature);
		return -1;
	}

	return 0;
}

int ey_preprocessor_detect_init(ey_engine_t *engine, engine_work_t *work)
{
	ey_assert(engine != NULL && work != NULL);
	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &engine->preprocessor_list, link)
	{
		if(pp->preprocessor_detect_init && pp->preprocessor_detect_init(engine, pp, work))
		{
			engine_runtime_error("init preprocessor work for %s failed\n", pp->name);
			return -1;
		}
		continue;
	}

	return 0;
}

void ey_preprocessor_detect_finit(ey_engine_t *engine, engine_work_t *work)
{
	ey_assert(engine != NULL && work != NULL);
	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &engine->preprocessor_list, link)
	{
		if(pp->preprocessor_detect_finit)
			pp->preprocessor_detect_finit(engine, pp, work);
		continue;
	}
}

int ey_preprocessor_detect(ey_engine_t *engine, engine_work_t *work, const char *buf, size_t buf_len, int from_client)
{
	ey_assert(engine != NULL && work != NULL);
	
	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &engine->preprocessor_list, link)
	{
		if(pp->preprocessor_detect)
			pp->preprocessor_detect(engine, pp, work, buf, buf_len, from_client);
		continue;
	}
	return 0;
}
