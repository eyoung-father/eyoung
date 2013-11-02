#ifndef LIBENGINE_EXPORT_H
#define LIBENGINE_EXPORT_H 1

#include "libengine_type.h"
#define ey_add_file_init(eng, func)									\
	_ey_add_file_init(eng, #func, func, __FILE__, __LINE__)
#define ey_add_file_finit(eng, func)								\
	_ey_add_file_finit(eng, #func, func, __FILE__, __LINE__)
extern int _ey_add_file_init(engine_t engine, const char *function, file_init_handle address, const char *filename, int line);
extern int _ey_add_file_finit(engine_t engine, const char *function, file_finit_handle address, const char *filename, int line);

#define ey_set_userdefine_work_init(eng,func)						\
	_ey_set_work_init(eng, 1, #func, func, __FILE__, __LINE__)
#define ey_set_predefine_work_init(eng,func)						\
	_ey_set_work_init(eng, 0, #func, func, __FILE__, __LINE__)
#define ey_set_userdefine_work_finit(eng,func)						\
	_ey_set_work_finit(eng, 1, #func, func, __FILE__, __LINE__)
#define ey_set_predefine_work_finit(eng,func)						\
	_ey_set_work_finit(eng, 0, #func, func, __FILE__, __LINE__)
extern int _ey_set_work_init(engine_t engine, int type, const char *function, work_init_handle address, const char *filename, int line);
extern int _ey_set_work_finit(engine_t engine, int type, const char *function, work_finit_handle address, const char *filename, int line);

#define ey_set_userdefine_event_init(eng,ev,func)					\
	_ey_set_event_init(eng, #ev, 1, #func, func, __FILE__, __LINE__)
#define ey_set_predefine_event_init(eng,ev,func)					\
	_ey_set_event_init(eng, #ev, 0, #func, func, __FILE__, __LINE__)
#define ey_set_userdefine_event_finit(eng,ev,func)					\
	_ey_set_event_finit(eng, #ev, 1, #func, func, __FILE__, __LINE__)
#define ey_set_predefine_event_finit(eng,ev,func)					\
	_ey_set_event_finit(eng, #ev, 0, #func, func, __FILE__, __LINE__)
extern int _ey_set_event_init(engine_t engine, const char *event, int type, 
	const char *function, event_init_handle address, const char *filename, int line);
extern int _ey_set_event_finit(engine_t engine, const char *event, int type, 
	const char *function, event_finit_handle address, const char *filename, int line);

#endif
