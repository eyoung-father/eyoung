#ifndef SNORT_INFO_H
#define SNORT_INFO_H 1

extern int debug_snort_parser;
extern int debug_snort_lexer;
extern int debug_snort_init;

extern int snort_parser_debug(const char *format, ...);
extern int snort_parser_error(const char *format, ...);

extern int snort_lexer_debug(const char *format, ...);
extern int snort_lexer_error(const char *format, ...);

extern int snort_init_debug(const char *format, ...);
extern int snort_init_error(const char *format, ...);
#endif
