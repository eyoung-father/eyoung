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

int ey_acsm_add_pattern(ey_acsm_t acsm, ey_acsm_pattern_t *pattern)
{
	return acsmAddPattern2((ACSM_STRUCT2*)acsm,
							(unsigned char*)(pattern->pattern),
							pattern->pattern_len,
							pattern->nocase,
							pattern->offset,
							pattern->depth,
							pattern->negative,
							pattern->id,
							0/*iid*/);
}

int ey_acsm_search(ey_acsm_t acsm, char *buf, int buf_len, acsm_match_fn callback, void *arg, int *last_state)
{
	return acsmSearch2((ACSM_STRUCT2*)acsm, (unsigned char*)buf, buf_len, callback, arg, last_state);
}
