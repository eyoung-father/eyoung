#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H 1

extern int debug_http_client_lexer;
extern int debug_http_server_lexer;
extern int debug_http_client_parser;
extern int debug_http_server_parser;
extern int debug_http_mem;
extern int debug_http_detect;

extern int http_debug(int flag, char *format, ...);
extern int http_abnormal(int flag, char *format, ...);
extern int http_attack(int flag, char *format, ...);
extern size_t http_parse_integer(const char *str, int mode, int *error);
extern int http_client_error(void *this_priv, const char *format, ...);
extern int http_server_error(void *this_priv, const char *format, ...);
extern char* http_string_trim(const char *old_string, size_t old_len, size_t *new_len);
#endif
