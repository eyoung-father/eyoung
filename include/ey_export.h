#ifndef EY_EXPORT_H
#define EY_EXPORT_H 1

struct eyoung_symbol
{
	unsigned long value;
	const char *name;
};

#define EY_EXPORT_IDENT(name,sym...)							\
	static const struct eyoung_symbol __eyoung_ident_##name		\
	__attribute__((section(".eyoung_ident"), unused))			\
	= { (unsigned long)&name, "extern " #sym ";"};	

#define EY_EXPORT_TYPE(name,sym...)								\
	sym															\
	static const struct eyoung_symbol __eyoung_type_##name		\
	__attribute__((section(".eyoung_type"), unused))			\
	= {0, #sym};
#endif 
