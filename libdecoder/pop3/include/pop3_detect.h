#ifndef POP3_DETECT
#define POP3_DETECT 1

extern int pop3_do_weak_password_check(void *priv_data);
extern int pop3_do_brute_force_check(void *priv_data);
extern int pop3_do_rule_detect(void *priv_data, const char* element_name, void *element_data);
extern int pop3_state_check(void *priv_data);
extern void pop3_state_transfer(void *priv_data);
#endif
