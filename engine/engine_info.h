#ifndef ENGINE_INFO_H
#define ENGINE_INFO_H 1

extern int debug_engine_parser;
extern int debug_engine_lexer;
extern int debug_engine_init;

extern int engine_parser_debug(const char *format, ...);
extern int engine_parser_error(const char *format, ...);

extern int engine_lexer_debug(const char *format, ...);
extern int engine_lexer_error(const char *format, ...);

extern int engine_init_debug(const char *format, ...);
extern int engine_init_error(const char *format, ...);
#endif
