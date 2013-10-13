#ifndef POP3_UTIL_H
#define POP3_UTIL_H 1

extern int debug_pop3_client;
extern int debug_pop3_server;
extern int debug_pop3_mem;
extern int debug_pop3_ips;

extern int pop3_debug(int flag, char *format, ...);
extern int pop3_abnormal(int flag, char *format, ...);
extern int pop3_attack(int flag, char *format, ...);
extern int pop3_parse_integer(char *str, int *error);
extern int pop3_client_error(void *this_priv, const char *format, ...);
extern int pop3_server_error(void *this_priv, const char *format, ...);
#endif
