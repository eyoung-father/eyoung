#ifndef ENGINE_LOCATION_H
#define ENGINE_LOCATION_H 1

#include "gram_parser.h"
typedef struct ey_location
{
	char *filename;
	int first_line;
	int first_column;
	int last_line;
	int last_column;
}ey_location_t;
#endif
