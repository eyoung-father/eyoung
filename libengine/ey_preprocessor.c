#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "ey_engine.h"

static ey_preprocessor_t *ey_preprocessor_find(ey_engine_t *engine, const char *name)
{
	assert(engine != NULL && name != NULL);
	
	ey_preprocessor_t *pp = NULL;
	TAILQ_FOREACH(pp, &ey_preprocessor_list(engine), link)
	{
		if(!strcmp(pp->name, name))
			break;
	}
	return pp;
}

ey_preprocessor_t *ey_preprocessor_register(ey_engine_t *engine, const char *name,
	preprocessor_init_fn init, preprocessor_load_fn load,
	preprocessor_detect_fn detect, preprocessor_finit_fn finit)
{
	ey_preprocessor_t *ret = NULL;
	assert(engine != NULL && name != NULL);
	
	if(ey_preprocessor_find(engine, name))
	{
		engine_init_error("preprocessor %s is already registered\n");
		return NULL;
	}

	ret = (ey_preprocessor_t*)engine_malloc(sizeof(ey_preprocessor_t));
	if(!ret)
	{
		engine_init_error("malloc preprocessor failed\n");
		return NULL;
	}

	memset(ret, 0, sizeof(*ret));

	strncpy(ret->name, name, sizeof(ret->name));
	ret->name[sizeof(ret->name)-1] = '\0';
	ret->preprocessor_init = init;
	ret->preprocessor_load = load;
	ret->preprocessor_detect = detect;
	ret->preprocessor_finit = finit;

	if(init && init(engine, ret))
	{
		engine_init_error("preprocessor init failed\n");
		engine_free(ret);
		return NULL;
	}

	TAILQ_INSERT_TAIL(&ey_preprocessor_list(engine), ret, link);
	return ret;
}

static void ey_preprocessor_unregister(ey_engine_t *engine, ey_preprocessor_t *preprocessor)
{
	assert(engine != NULL && preprocessor != NULL);

	if(preprocessor->preprocessor_finit)
		preprocessor->preprocessor_finit(engine, preprocessor);
	
	engine_free(preprocessor);
}

void ey_preprocessor_finit(ey_engine_t *engine)
{
	assert(engine != NULL);

	ey_preprocessor_t *pp = NULL, *tmp = NULL;
	TAILQ_FOREACH_SAFE(pp, &ey_preprocessor_list(engine), link, tmp)
		ey_preprocessor_unregister(engine, pp);
	
	TAILQ_INIT(&ey_preprocessor_list(engine));
}

int ey_preprocessor_init(ey_engine_t *engine)
{
	assert(engine != NULL);

	TAILQ_INIT(&ey_preprocessor_list(engine));
	return 0;
}
