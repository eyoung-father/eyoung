#include "pop3_type.h"
#include "pop3_detect.h"
#include "pop3_util.h"

int pop3_do_weak_password_check(struct pop3_data *priv_data)
{
	pop3_debug(debug_pop3_ips, "do weak password check\n");
	return 0;
}

int pop3_do_brute_force_check(struct pop3_data *priv_data)
{
	pop3_debug(debug_pop3_ips, "do auth brute force check\n");
	return 0;
}

int pop3_do_rule_detect(struct pop3_data *priv_data, const char* element_name, void *element_data)
{
	pop3_debug(debug_pop3_ips, "do signature check for %s\n", element_name);
	return 0;
}
