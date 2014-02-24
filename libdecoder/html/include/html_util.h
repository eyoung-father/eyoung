#ifndef HTML_UTIL_H
#define HTML_UTIL_H 1

extern int debug_html_lexer;
extern int debug_html_parser;
extern int debug_html_mem;
extern int debug_html_detect;

extern int ey_html_debug(int flag, char *format, ...);
extern int ey_html_abnormal(int flag, char *format, ...);
extern int ey_html_attack(int flag, char *format, ...);
extern int html_error(void *this_priv, const char *format, ...);
#endif
