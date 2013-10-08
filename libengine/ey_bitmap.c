#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "engine_mem.h"
#include "ey_bitmap.h"
#include "ey_info.h"
#include "ey_engine.h"

int ey_bitmap_init(ey_engine_t *eng)
{
	char name[64];
	if(!ey_bitmap_slab(eng))
	{
		snprintf(name, sizeof(name), "%s bitmap slab", eng->name);
		name[63] = 0;
		ey_bitmap_slab(eng) = engine_zinit(name, sizeof(ey_bitmap_t));
		if(!ey_bitmap_slab(eng))
		{
			engine_init_error("init bitmap failed\n");
			return -1;
		}
	}

	if(!ey_bitmap_buffer_fslab(eng))
	{
		snprintf(name, sizeof(name), "%s bitmap memory fslab", eng->name);
		name[63] = 0;
		ey_bitmap_buffer_fslab(eng) = engine_fzinit(name, 512, NULL);
		if(!ey_bitmap_buffer_fslab(eng))
		{
			engine_init_error("init bitmap memory buffer failed\n");
			return -1;
		}
	}

	return 0;
}

void ey_bitmap_finit(ey_engine_t *eng)
{
	engine_fzfinit(ey_bitmap_buffer_fslab(eng));
	engine_zfinit(ey_bitmap_slab(eng));
}

ey_bitmap_t *ey_bitmap_create(ey_engine_t *eng, unsigned long count)
{
	ey_assert(ey_bitmap_buffer_fslab(eng) != NULL);
	ey_assert(ey_bitmap_slab(eng) != NULL);

	unsigned long nbytes = (count>>3);
	if(count & 7)
		nbytes++;
	
	char *buffer = engine_fzalloc(nbytes, ey_bitmap_buffer_fslab(eng));
	if(!buffer)
	{
		engine_runtime_error("malloc bitmap buffer failed\n");
		return NULL;
	}
	memset(buffer, 0, nbytes);

	ey_bitmap_t *ret = engine_zalloc(ey_bitmap_slab(eng));
	if(!ret)
	{
		engine_runtime_error("alloc bitmap failed\n");
		engine_fzfree(ey_bitmap_buffer_fslab(eng), buffer);
		return NULL;
	}
	ret->bitmap = buffer;
	ret->count = count;
	ret->size = (nbytes<<3);
	return ret;
}

void ey_bitmap_destroy(ey_engine_t *eng, ey_bitmap_t *bitmap)
{
	ey_assert(ey_bitmap_buffer_fslab(eng) != NULL);
	ey_assert(ey_bitmap_slab(eng) != NULL);

	if(!bitmap)
		return;
	
	if(bitmap->bitmap)
		engine_fzfree(ey_bitmap_buffer_fslab(eng), bitmap->bitmap);
	engine_zfree(ey_bitmap_slab(eng), bitmap);
}

static unsigned char set_mask[8] = 
{
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

void ey_bitmap_set(ey_engine_t *eng, ey_bitmap_t *bitmap, unsigned long nbit)
{
	ey_assert(bitmap != NULL);
	ey_assert(bitmap->count >= nbit);

	unsigned long byte_index = (nbit >> 3);
	unsigned long bit_index = (nbit & 7);
	unsigned char bit_mask = set_mask[bit_index];
	bitmap->bitmap[byte_index] |= bit_mask;
}

void ey_bitmap_clear(ey_engine_t *eng, ey_bitmap_t *bitmap, unsigned long nbit)
{
	ey_assert(bitmap != NULL);
	ey_assert(bitmap->count >= nbit);

	unsigned long byte_index = (nbit >> 3);
	unsigned long bit_index = (nbit & 7);
	unsigned char bit_mask = ~set_mask[bit_index];
	bitmap->bitmap[byte_index] &= bit_mask;
}

int ey_bitmap_isset(ey_engine_t *eng, ey_bitmap_t *bitmap, unsigned long nbit)
{
	ey_assert(bitmap != NULL);
	ey_assert(bitmap->count >= nbit);

	unsigned long byte_index = (nbit >> 3);
	unsigned long bit_index = (nbit & 7);
	unsigned char bit_mask = set_mask[bit_index];
	return (bitmap->bitmap[byte_index] & bit_mask);
}
