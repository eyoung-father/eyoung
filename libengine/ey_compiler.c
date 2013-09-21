#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "ey_engine.h"
#include "ey_compiler.h"

int ey_compiler_init(struct ey_engine *eng)
{
	ey_jit_t jit = ey_jit_create(NULL, NULL, NULL);
	if(!jit)
	{
		engine_init_error("create JIT compiler failed\n");
		goto failed;
	}

	if(ey_jit_add_sysinclude_path(jit, SYSTEM_INCLUDE_PATH))
	{
		engine_init_error("add system include path failed\n");
		goto failed;
	}

	if(ey_jit_add_include_path(jit, LOCAL_INCLUDE_PATH))
	{
		engine_init_error("add local include path failed\n");
		goto failed;
	}

	if(ey_jit_add_library_path(jit, SYSTEM_LIBRARY_PATH))
	{
		engine_init_error("add system library path failed\n");
		goto failed;
	}

	if(ey_jit_add_library_path(jit, LOCAL_LIBRARY_PATH))
	{
		engine_init_error("add local library path failed\n");
		goto failed;
	}

	ey_jit(eng) = jit;
	return 0;

failed:
	if(jit)
		ey_jit_destroy(jit);

	return -1;
}

void ey_compiler_finit(struct ey_engine *eng)
{
	if(!eng)
		return;

	if(ey_jit(eng))
		ey_jit_destroy(eng->jit);
	ey_jit(eng) = NULL;
}

int ey_compile_signature_file(ey_engine_t *eng, ey_signature_file_t *file, int need_relocate)
{
	if(!eng || !ey_jit(eng) || !file || !file->output_file)
	{
		engine_compiler_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	if(ey_jit_compile_file(ey_jit(eng), file->output_file))
	{
		engine_compiler_error("compile %s failed\n", file->output_file);
		return -1;
	}
	
	if(need_relocate)
	{
		if(ey_jit_relocate(ey_jit(eng), EY_JIT_RELOCATE_AUTO))
		{
			engine_compiler_error("link file %s failed\n", file->output_file);
			return -1;
		}

		ey_signature_t *signature = NULL;
		TAILQ_FOREACH(signature, &file->signature_list, link)
		{
			ey_rhs_signature_t *rhs_signature = NULL;
			TAILQ_FOREACH(rhs_signature, &signature->rhs_signature_list, link)
			{
				ey_rhs_item_t *item = NULL;
				TAILQ_FOREACH(item, &rhs_signature->rhs_item_list, link)
				{
					ey_rhs_item_condition_t *condition = item->condition;
					if(condition && condition->raw_code && condition->func_name)
					{
						condition->addr = ey_jit_get_symbol(ey_jit(eng), condition->func_name);
						if(!condition->addr)
						{
							engine_compiler_error("relocate condition function %s[%s:%d] failed\n", condition->func_name,
								condition->location.filename, condition->location.first_line);
							return -1;
						}
					}

					ey_rhs_item_action_t *action = item->action;
					if(action && action->raw_code && action->func_name)
					{
						action->addr = ey_jit_get_symbol(ey_jit(eng), action->func_name);
						if(!action->addr)
						{
							engine_compiler_error("relocate action function %s[%s:%d] failed\n", action->func_name,
								action->location.filename, action->location.first_line);
							return -1;
						}
					}
				}
			}
		}
	}

	return 0;
}

int ey_compile_string(ey_engine_t *eng, const char *string)
{
	if(!eng || !ey_jit(eng) || !string)
	{
		engine_compiler_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	if(ey_jit_compile_string(ey_jit(eng), string))
	{
		engine_compiler_error("compile %s failed\n", string);
		return -1;
	}

	if(ey_jit_relocate(ey_jit(eng), EY_JIT_RELOCATE_AUTO))
	{
		engine_compiler_error("link string %s failed\n", string);
		return -1;
	}

	return 0;
}

int ey_compiler_link(ey_engine_t *eng)
{
	if(!eng || !ey_jit(eng))
	{
		engine_compiler_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	if(ey_jit_relocate(ey_jit(eng), EY_JIT_RELOCATE_AUTO))
	{
		engine_compiler_error("link signature files\n");
		return -1;
	}

	ey_signature_t *signature = NULL;
	TAILQ_FOREACH(signature, &eng->signature_list, link)
	{
		ey_rhs_signature_t *rhs_signature = NULL;
		TAILQ_FOREACH(rhs_signature, &signature->rhs_signature_list, link)
		{
			ey_rhs_item_t *item = NULL;
			TAILQ_FOREACH(item, &rhs_signature->rhs_item_list, link)
			{
				ey_rhs_item_condition_t *condition = item->condition;
				if(condition && condition->raw_code && condition->func_name)
				{
					condition->addr = ey_jit_get_symbol(ey_jit(eng), condition->func_name);
					if(!condition->addr)
					{
						engine_compiler_error("relocate condition function %s[%s:%d] failed\n", condition->func_name,
							condition->location.filename, condition->location.first_line);
						return -1;
					}
				}

				ey_rhs_item_action_t *action = item->action;
				if(action && action->raw_code && action->func_name)
				{
					action->addr = ey_jit_get_symbol(ey_jit(eng), action->func_name);
					if(!action->addr)
					{
						engine_compiler_error("relocate action function %s[%s:%d] failed\n", action->func_name,
							action->location.filename, action->location.first_line);
						return -1;
					}
				}
			}
		}
	}

	engine_compiler_debug("link all signature files failed\n");
	return 0;
}
