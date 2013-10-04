#ifndef LIBENGINE_TYPE_H
#define LIBENGINE_TYPE_H 1

typedef void* engine_t;
typedef int (*file_init_handle)(engine_t eng);
typedef int (*file_finit_handle)(engine_t eng);

typedef struct engine_work
{
	unsigned long work_id;
	engine_t engine;
	void *priv_data;	/*for libengine itself*/
	void *predefined;	/*for protocol parser*/
	void *user_defined;	/*for signature writer*/
}engine_work_t;
typedef int (*work_init_handle)(engine_work_t *work);
typedef int (*work_finit_handle)(engine_work_t *work);

typedef struct engine_work_event
{
	unsigned long event_id;
	engine_work_t *work;
	void *predefined;	/*for protocol parser*/
	void *user_defined;	/*for signature writer*/
}engine_work_event_t;
typedef int (*event_init_handle)(engine_work_event_t *work_event);
typedef int (*event_finit_handle)(engine_work_event_t *work_event);

typedef struct engine_action
{
	unsigned int action;
}engine_action_t;

#endif
