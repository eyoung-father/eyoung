#ifndef LIBENGINE_TYPE_H
#define LIBENGINE_TYPE_H 1

#include "ey_queue.h"

typedef void* engine_t;
typedef int (*file_init_handle)(engine_t eng);
typedef int (*file_finit_handle)(engine_t eng);

typedef struct engine_work
{
	TAILQ_ENTRY(engine_work) link;
	engine_t engine;
	void *priv_data;	/*for libengine itself*/
	void *predefined;	/*for protocol parser*/
	void *user_defined;	/*for signature writer*/
}engine_work_t;
typedef TAILQ_HEAD(engine_work_list, engine_work) engine_work_list_t;
typedef int (*work_init_handle)(engine_work_t *work);
typedef int (*work_finit_handle)(engine_work_t *work);

typedef struct engine_action
{
	unsigned int action;
}engine_action_t;

typedef struct engine_work_event
{
	TAILQ_ENTRY(engine_work_event) link;
	engine_work_t *work;
	void *event;
	void *predefined;	/*for protocol parser*/
	void *user_defined;	/*for signature writer*/
	engine_action_t *action;	/*OUTPUT*/
}engine_work_event_t;
typedef TAILQ_HEAD(engine_work_event_list, engine_work_event) engine_work_event_list_t;
typedef int (*event_init_handle)(engine_work_event_t *work_event);
typedef int (*event_finit_handle)(engine_work_event_t *work_event);

#endif
