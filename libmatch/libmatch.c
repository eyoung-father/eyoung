#include "libmatch.h"
#include "acsmx2.h"

ey_acsm_t ey_acsm_create()
{
	return (ey_acsm_t)acsmNew2(NULL, NULL, NULL);
}

void ey_acsm_destroy(ey_acsm_t acsm)
{
	acsmFree2((ACSM_STRUCT2*)acsm);
}

int ey_acsm_add_pattern(ey_acsm_t acsm, char *pattern, int pattern_len, void *id, ey_acsm_option_t *option)
{
	return acsmAddPattern2((ACSM_STRUCT2*)acsm,
							(unsigned char*)pattern,
							pattern_len,
							option?option->nocase:0,
							option?option->offset:0,
							option?option->depth:0,
							option?option->negative:0,
							id,
							0/*iid*/);
}

int ey_acsm_search(ey_acsm_t acsm, char *buf, int buf_len, acsm_match_fn callback, void *arg, int *last_state)
{
	return acsmSearch2((ACSM_STRUCT2*)acsm, (unsigned char*)buf, buf_len, callback, arg, last_state);
}
