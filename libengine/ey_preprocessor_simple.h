#ifndef EY_PREPROCESSOR_SIMPLE_H
#define EY_PREPROCESSOR_SIMPLE_H 1

struct ey_engine;

typedef struct ey_simple_private
{
	int client_last_state;
	int server_last_state;
}ey_simple_private_t;

extern int ey_preprocessor_simple_register(struct ey_engine *engine);
#endif
