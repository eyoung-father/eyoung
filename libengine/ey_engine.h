#ifndef EY_ENGINE_H
#define EY_ENGINE_H 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <dlfcn.h>

#include "ey_memory.h"
#include "ey_event.h"
#include "ey_parser.h"
#include "ey_info.h"
#include "engine_mem.h"
#include "ey_signature.h"
#include "libutil.h"
#include "libjit.h"
#include "ey_compiler.h"
#include "ey_import.h"
#include "ey_elf.h"
#include "libmatch.h"

typedef struct ey_engine
{
	char name[64];

	ey_fslab_t parser_fslab;

	ey_hash_t filename_hash;
	ey_hash_t signature_hash;
	ey_hash_t library_hash;

	#define EVENT_ARRAY_STEP	32
	ey_event_t *event_array;
	int event_size;
	int event_count;

	ey_parser_t *parser;
	ey_jit_t jit;

	unsigned int rhs_id;
}ey_engine_t;

#define ey_parser_fslab(eng) (((ey_engine_t*)(eng))->parser_fslab)
#define ey_filename_hash(eng) (((ey_engine_t*)(eng))->filename_hash)
#define ey_event_array(eng) (((ey_engine_t*)(eng))->event_array)
#define ey_event_size(eng) (((ey_engine_t*)(eng))->event_size)
#define ey_event_count(eng) (((ey_engine_t*)(eng))->event_count)
#define ey_signature_hash(eng) (((ey_engine_t*)(eng))->signature_hash)
#define ey_jit(eng) (((ey_engine_t*)(eng))->jit)
#define ey_library_hash(eng) (((ey_engine_t*)(eng))->library_hash)
#define ey_rhs_id(eng) (((ey_engine_t*)(eng))->rhs_id)

#endif
