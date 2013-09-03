#ifndef EY_ENGINE_H
#define EY_ENGINE_H 1

#include <stdio.h>
#include "ey_memory.h"
#include "ey_parser.h"
#include "ey_info.h"
#include "engine_mem.h"
typedef struct ey_engine
{
	char name[64];

	ey_fslab_t filename_fslab;
	ey_hash_t filename_hash;
}ey_engine_t;

#define ey_filename_fslab(eng) (((ey_engine_t*)(eng))->filename_fslab)
#define ey_filename_hash(eng) (((ey_engine_t*)(eng))->filename_hash)

#endif
