#ifndef POP3_DETECT
#define POP3_DETECT 1

typedef enum pop3_state
{
	POP3_STATE_INIT=0,
	POP3_STATE_AUTHORIZATION,
	POP3_STATE_TRANSACTION,
	POP3_STATE_UPDATE,

	POP3_STATE_MAX
}pop3_state_t;

static inline const char *pop3_state_name(pop3_state_t state)
{
	switch(state)
	{
		case POP3_STATE_INIT:
			return "INIT";
		case POP3_STATE_AUTHORIZATION:
			return "AUTH";
		case POP3_STATE_TRANSACTION:
			return "TRANSACTION";
		case POP3_STATE_UPDATE:
			return "UPDATE";
		case POP3_STATE_MAX:
		default:
			return "Unknown";
	}
}

extern int pop3_do_weak_password_check(void *priv_data);
extern int pop3_do_brute_force_check(void *priv_data);
extern int pop3_state_check(void *priv_data);
extern void pop3_state_transfer(void *priv_data);
#endif
