#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "engine_mem.h"
#include "ey_bitmap.h"
#include "ey_info.h"

static ey_slab_t bitmap_slab;
static ey_fslab_t bitmap_buffer_fslab;

int ey_bitmap_init()
{
	bitmap_slab = engine_zinit("bitmap slab", sizeof(ey_bitmap_t));
	if(!bitmap_slab)
	{
		engine_init_error("init bitmap failed\n");
		return -1;
	}

	bitmap_buffer_fslab = engine_fzinit("bitmap memory fslab", 512, NULL);
	if(!bitmap_buffer_fslab)
	{
		engine_init_error("init bitmap memory buffer failed\n");
		return -1;
	}

	return 0;
}

void ey_bitmap_finit()
{
	engine_fzfinit(bitmap_buffer_fslab);
	engine_zfinit(bitmap_slab);
}

ey_bitmap_t *ey_bitmap_create(unsigned int count)
{
	assert(bitmap_buffer_fslab != NULL);
	assert(bitmap_slab != NULL);

	unsigned int nbytes = (count>>3);
	if(count & 7)
		nbytes++;
	
	char *buffer = engine_fzalloc(nbytes, bitmap_buffer_fslab);
	if(!buffer)
	{
		engine_runtime_error("malloc bitmap buffer failed\n");
		return NULL;
	}
	memset(buffer, 0, nbytes);

	ey_bitmap_t *ret = engine_zalloc(bitmap_slab);
	if(!ret)
	{
		engine_runtime_error("alloc bitmap failed\n");
		engine_fzfree(bitmap_buffer_fslab, buffer);
		return NULL;
	}
	ret->bitmap = buffer;
	ret->count = count;
	ret->size = (nbytes<<3);
	return ret;
}

void ey_bitmap_destroy(ey_bitmap_t *bitmap)
{
	assert(bitmap_buffer_fslab != NULL);
	assert(bitmap_slab != NULL);

	if(!bitmap)
		return;
	
	if(bitmap->bitmap)
		engine_fzfree(bitmap_buffer_fslab, bitmap->bitmap);
	engine_zfree(bitmap_slab, bitmap);
}

static unsigned char set_mask[8] = 
{
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};
void ey_bitmap_set(ey_bitmap_t *bitmap, unsigned int nbit)
{
	assert(bitmap != NULL);
	assert(bitmap->count >= nbit);

	unsigned int byte_index = (nbit >> 3);
	unsigned int bit_index = (nbit & 7);
	unsigned char bit_mask = set_mask[bit_index];
	bitmap->bitmap[byte_index] |= bit_mask;
}

void ey_bitmap_clear(ey_bitmap_t *bitmap, unsigned int nbit)
{
	assert(bitmap != NULL);
	assert(bitmap->count >= nbit);

	unsigned int byte_index = (nbit >> 3);
	unsigned int bit_index = (nbit & 7);
	unsigned char bit_mask = ~set_mask[bit_index];
	bitmap->bitmap[byte_index] &= bit_mask;
}

int ey_bitmap_isset(ey_bitmap_t *bitmap, unsigned int nbit)
{
	assert(bitmap != NULL);
	assert(bitmap->count >= nbit);

	unsigned int byte_index = (nbit >> 3);
	unsigned int bit_index = (nbit & 7);
	unsigned char bit_mask = set_mask[bit_index];
	return (bitmap->bitmap[byte_index] & bit_mask);
}
