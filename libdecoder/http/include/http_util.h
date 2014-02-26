#ifndef HTTP_UTIL_H
#define HTTP_UTIL_H 1

#include "http_type.h"

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
extern http_body_content_encoding_t http_parse_content_encoding(const char *value);
extern http_body_content_language_t http_parse_content_language(const char *value);
extern void http_parse_content_type(const char *value, http_body_content_maintype_t *main_type, 
									http_body_content_subtype_t *sub_type, 
									http_body_content_charset_t *charset);
#endif
