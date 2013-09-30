#ifndef LIBENGINE_TYPE_H
#define LIBENGINE_TYPE_H 1

typedef void* engine_t;

typedef struct engine_work_event
{
	unsigned long event_id;
	void *predefined;
	void *user_defined;
}engine_work_event_t;

typedef struct engine_work
{
	unsigned long work_id;
	void *predefined;
	void *user_defined;
}engine_work_t;

typedef struct engine_action
{
	unsigned int action;
}engine_action_t;

#endif
