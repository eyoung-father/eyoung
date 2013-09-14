#ifndef EY_LOC_H
#define EY_LOC_H 1

typedef struct GRAM_LTYPE
{
	int first_line;
	int first_column;
	int last_line;
	int last_column;
	const char *filename;
} GRAM_LTYPE;
typedef GRAM_LTYPE ey_location_t;
#endif
