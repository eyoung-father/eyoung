#ifndef LIBMATCH_H
#define LIBMATCH_H 1

typedef void* ey_acsm_t;
typedef int (*acsm_match_fn)(void *id, void *tree, int index, void *data, void *neg_list);

typedef struct ey_acsm_pattern
{
	char *pattern;
	int pattern_len;
	void *id;
	int nocase;
	int offset;
	int depth;
	int negative;
}ey_acsm_pattern_t;

extern ey_acsm_t ey_acsm_create();
extern void ey_acsm_destroy(ey_acsm_t acsm);
extern int ey_acsm_add_pattern(ey_acsm_t acsm, ey_acsm_pattern_t *option);
extern int ey_acsm_search(ey_acsm_t acsm, char *buf, int buf_len, acsm_match_fn callback, void *arg, int *last_state);
#endif
