#ifndef POP3_DETECT
#define POP3_DETECT 1

struct pop3_data;
extern int pop3_do_weak_password_check(struct pop3_data *priv_data);
extern int pop3_do_brute_force_check(struct pop3_data *priv_data);
extern int pop3_do_rule_detect(struct pop3_data *priv_data, const char* element_name, void *element_data);
#endif
