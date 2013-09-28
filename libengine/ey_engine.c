#include "ey_engine.h"

int ey_load_post_action(ey_engine_t *eng)
{
	if(!eng || !ey_rhs_id(eng))
	{
		engine_init_error("%s bad parameter\n", __FUNCTION__);
		return -1;
	}

	/*alloc prefix and postfix array*/
	unsigned int *ptr = (unsigned int*)engine_fzalloc(sizeof(unsigned int)*ey_rhs_id(eng)*2, ey_parser_fslab(eng));
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
