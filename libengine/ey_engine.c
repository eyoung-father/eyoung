#include "ey_engine.h"

static int do_link(ey_engine_t *eng)
{
	if(ey_jit_relocate(ey_jit(eng), EY_JIT_RELOCATE_AUTO))
	{
		engine_compiler_error("link signatures failed\n");
		return -1;
	}
	
	/*set condition and action function pointer*/
	ey_signature_t *signature = NULL;
	TAILQ_FOREACH(signature, &ey_signature_list(eng), link)
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
					assert(condition->addr == NULL);
					condition->addr = ey_jit_get_symbol(ey_jit(eng), condition->func_name);
					if(!condition->addr)
					{
						engine_compiler_error("relocate condition function %s[%s:%d] failed\n", condition->func_name,
							condition->location.filename, condition->location.first_line);
						return -1;
					}
					engine_compiler_debug("compile %s, address: %p\n", condition->func_name, condition->addr);
				}

				ey_rhs_item_action_t *action = item->action;
				if(action && action->raw_code && action->func_name)
				{
					assert(action->addr == NULL);
					action->addr = ey_jit_get_symbol(ey_jit(eng), action->func_name);
					if(!action->addr)
					{
						engine_compiler_error("relocate action function %s[%s:%d] failed\n", action->func_name,
							action->location.filename, action->location.first_line);
						return -1;
					}
					engine_compiler_debug("compile %s, address: %p\n", action->func_name, action->addr);
				}
			}
		}
	}

	/*set file init/finit function, and call init function*/
	ey_code_t *function = NULL;
	TAILQ_FOREACH(function, &ey_file_init_list(eng), link)
	{
		assert(function->handle == NULL);
		function->handle = ey_jit_get_symbol(ey_jit(eng), function->function);
		if(!function->handle)
		{
			engine_compiler_error("relocate file init function %s[%s:%d] failed\n", function->function,
				function->location.filename, function->location.first_line);
			return -1;
		}
		engine_compiler_debug("compile %s, address: %p\n", function->function, function->handle);

		if(((init_handler)(function->handle))(eng))
		{
			engine_compiler_error("file init function %s[%s,%d] run failed\n", function->function,
				function->location.filename, function->location.first_line);
			return -1;
		}
	}

	TAILQ_FOREACH(function, &ey_file_finit_list(eng), link)
	{
		function->handle = ey_jit_get_symbol(ey_jit(eng), function->function);
		if(!function->handle)
		{
			engine_compiler_error("relocate file finit function %s[%s:%d] failed\n", function->function,
				function->location.filename, function->location.first_line);
			return -1;
		}
		engine_compiler_debug("compile %s, address: %p\n", function->function, function->handle);
	}

	/*set work init/finit function*/
	function = ey_work_init_userdefined(eng);
	if(function && !function->handle)
	{
		function->handle = ey_jit_get_symbol(ey_jit(eng), function->function);
		if(!function->handle)
		{
			engine_compiler_error("relocate work init function %s[%s:%d] failed\n", function->function,
				function->location.filename, function->location.first_line);
			return -1;
		}
		engine_compiler_debug("compile %s, address: %p\n", function->function, function->handle);
	}

	function = ey_work_finit_userdefined(eng);
	if(function && !function->handle)
	{
		function->handle = ey_jit_get_symbol(ey_jit(eng), function->function);
		if(!function->handle)
		{
			engine_compiler_error("relocate work finit function %s[%s:%d] failed\n", function->function,
				function->location.filename, function->location.first_line);
			return -1;
		}
		engine_compiler_debug("compile %s, address: %p\n", function->function, function->handle);
	}

	/*set event init/finit function*/
	int index;
	ey_event_t *event = NULL;
	for(index=0, event=ey_event_array(eng); index<ey_event_count(eng); index++, event++)
	{
		engine_compiler_debug("compile functions for event %s\n", event->name);
		function = event->event_init_userdefined;
		if(function)
		{
			if(function->handle)
				continue;
			function->handle = ey_jit_get_symbol(ey_jit(eng), function->function);
			if(!function->handle)
			{
				engine_compiler_error("relocate event %s init function %s[%s:%d] failed\n", event->name, 
					function->function, function->location.filename, function->location.first_line);
				return -1;
			}
			engine_compiler_debug("compile %s, address: %p\n", function->function, function->handle);
		}

		function = event->event_finit_userdefined;
		if(function)
		{
			if(function->handle)
				continue;
			function->handle = ey_jit_get_symbol(ey_jit(eng), function->function);
			if(!function->handle)
			{
				engine_compiler_error("relocate event %s finit function %s[%s:%d] failed\n", event->name,
					function->function, function->location.filename, function->location.first_line);
				return -1;
			}
			engine_compiler_debug("compile %s, address: %p\n", function->function, function->handle);
		}
	}
	engine_compiler_debug("do link successfully\n");
	return 0;
}

static int do_prefix_postfix(ey_engine_t *eng)
{
	/*alloc prefix and postfix array*/
	unsigned long *ptr = (unsigned long*)engine_fzalloc(sizeof(unsigned long)*ey_rhs_id(eng)*2, ey_parser_fslab(eng));
	if(!ptr)
	{
		engine_init_error("alloc prefix and postfix array failed\n");
		return -1;
	}
	ey_prefix_array(eng) = ptr;
	ey_postfix_array(eng) = ptr + ey_rhs_id(eng);

	/*set prefix and postfix array*/
	ey_signature_t *signature = NULL;
	TAILQ_FOREACH(signature, &ey_signature_list(eng), link)
	{
		ey_rhs_signature_t *rhs_signature = NULL;
		TAILQ_FOREACH(rhs_signature, &signature->rhs_signature_list, link)
		{
			int prefix = 0;
			ey_rhs_item_t *item = NULL;
			TAILQ_FOREACH(item, &rhs_signature->rhs_item_list, link)
			{
				ey_prefix_array(eng)[item->rhs_id] = prefix;
				ey_postfix_array(eng)[item->rhs_id] = ((TAILQ_NEXT(item,link)) ? (TAILQ_NEXT(item,link)->rhs_id) : (0));
				prefix = item->rhs_id;
			}
		}
	}
	engine_init_debug("set prefix and postfix array ok\n");
	return 0;
}

static int do_acsm_compile(ey_engine_t *eng)
{
	/*do acsm compile for all event*/
	if(ey_event_array(eng) && ey_event_count(eng))
	{
		int index = 0;
		for(index=0; index<ey_event_count(eng); index++)
		{
			assert(ey_event_array(eng)[index].cluster_pattern != NULL);
			if(ey_acsm_compile(ey_event_array(eng)[index].cluster_pattern))
			{
				engine_init_error("compile acsm for event %s failed\n", ey_event_array(eng)[index].name);
				return -1;
			}
		}
	}
	engine_init_debug("set compile event acsm ok\n");
	return 0;
}

int ey_load_post_action(ey_engine_t *eng)
{
	if(!eng || !ey_rhs_id(eng) || !ey_jit(eng))
	{
		engine_init_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}
	
	/*relocate*/
	if(do_link(eng))
		return -1;
	
	if(do_prefix_postfix(eng))
		return -1;
	
	if(do_acsm_compile(eng))
		return -1;
	
	return 0;
}
