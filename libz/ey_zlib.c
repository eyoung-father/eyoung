#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "ey_memory.h"
#include "ey_zlib.h"
#include "zlib.h"
#include "zutil.h"

#define DEFAULT_INPUT_LENGTH	1024
#define DEFAULT_OUTPUT_LENGTH	2048
typedef struct ey_zlib_private
{
	memory_handler_t memory_handler;
	ey_zlib_format_t format;
	size_t input_length;
	size_t output_length;
	void *create_arg;
	void *runtime_arg;
	char *err_msg;
	z_stream zstream;
}ey_zlib_private_t;

static ey_slab_t zlib_slab;
static int debug_zlib_basic = 1;

static void zlib_debug(int flag, const char *format, ...)
{
	if(!flag)
		return;
	
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	va_end(ap);
}

static void *ey_zlib_alloc(void *arg, unsigned int n, unsigned int size)
{
	assert(arg != NULL);
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)arg;
	size_t total_len = n * size;
	return priv_data->memory_handler.malloc(total_len);
}

static void ey_zlib_free(void *arg, void *ptr)
{
	assert(arg != NULL);
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)arg;
	if(ptr)
		priv_data->memory_handler.free(ptr);
}

static int ey_zlib_zstream_init(z_stream *sp, ey_zlib_format_t fmt)
{
	if(!sp)
	{
		zlib_debug(debug_zlib_basic, "null z_stream, init failed\n");
		return -1;
	}
	
	switch(fmt)
	{
		case EY_ZLIB_FORMAT_GZIP_UNPACK:
		{
			if(inflateInit2(sp, MAX_WBITS + 16) != Z_OK)
			{
				zlib_debug(debug_zlib_basic, "init GZIP format uncompress failed: %s\n", sp->msg?sp->msg:"unkown reason");
				return -1;
			}
			break;
		}
		case EY_ZLIB_FORMAT_DEFLATE_UNPACK:
		{
			if(inflateInit(sp) != Z_OK)
			{
				zlib_debug(debug_zlib_basic, "init DEFLATE format uncompress failed: %s\n", sp->msg?sp->msg:"unkown reason");
				return -1;
			}
			break;
		}
		case EY_ZLIB_FORMAT_DEFLATE_PACK:
		{
			if(deflateInit(sp, Z_DEFAULT_COMPRESSION) != Z_OK)
			{
				zlib_debug(debug_zlib_basic, "init DEFLATE format compress failed: %s\n", sp->msg?sp->msg:"unkown reason");
				return -1;
			}
			break;
		}
		case EY_ZLIB_FORMAT_GZIP_PACK:
		{
			if(deflateInit2(sp, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS + 16, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY) != Z_OK)
			{
				zlib_debug(debug_zlib_basic, "init GZIP format compress failed: %s\n", sp->msg?sp->msg:"unkown reason");
				return -1;
			}
			break;
		}
		default:
		{
			zlib_debug(debug_zlib_basic, "unkown format\n");
			return -1;
		}
	}
	zlib_debug(debug_zlib_basic, "init successfully\n");
	return 0;
}

static void ey_zlib_zstream_finit(z_stream *sp, ey_zlib_format_t fmt)
{
	if(!sp)
	{
		zlib_debug(debug_zlib_basic, "null z_stream, finit failed\n");
		return;
	}
	
	switch(fmt)
	{
		case EY_ZLIB_FORMAT_GZIP_UNPACK:
		case EY_ZLIB_FORMAT_DEFLATE_UNPACK:
		{
			inflateEnd(sp);
			break;
		}
		case EY_ZLIB_FORMAT_DEFLATE_PACK:
		case EY_ZLIB_FORMAT_GZIP_PACK:
		{
			deflateEnd(sp);
			break;
		}
		default:
		{
			zlib_debug(debug_zlib_basic, "unkown format\n");
			return;
		}
	}
	zlib_debug(debug_zlib_basic, "finit successfully\n");
}

ey_zlib_t ey_zlib_create(memory_handler_t *mm, size_t ilen, size_t olen, ey_zlib_format_t fmt, void *arg)
{
	memory_handler_t mm_handler = {ey_malloc,ey_realloc,ey_free,ey_calloc};
	char *i_buf = NULL;
	char *o_buf = NULL;
	ey_zlib_private_t *ret = NULL;
	z_stream *sp = NULL;
	
	if(!mm)
		mm = &mm_handler;
	if(ilen < DEFAULT_INPUT_LENGTH)
		ilen = DEFAULT_INPUT_LENGTH;
	if(olen < DEFAULT_OUTPUT_LENGTH)
		olen = DEFAULT_OUTPUT_LENGTH;
	
	assert(	fmt==EY_ZLIB_FORMAT_GZIP_PACK		|| 
			fmt==EY_ZLIB_FORMAT_GZIP_UNPACK		|| 
			fmt==EY_ZLIB_FORMAT_DEFLATE_PACK	||
			fmt==EY_ZLIB_FORMAT_DEFLATE_UNPACK);

	if(!zlib_slab)
	{
		zlib_slab = ey_zinit("ey_zlib_slab", sizeof(ey_zlib_private_t), NULL);
		if(!zlib_slab)
		{
			zlib_debug(debug_zlib_basic, "zlib slab create failed\n");
			goto failed;
		}
	}

	ret = (ey_zlib_private_t*)ey_zalloc(zlib_slab);
	if(!ret)
	{
		zlib_debug(debug_zlib_basic, "alloc zlib_private data failed\n");
		goto failed;
	}
	memset(ret, 0, sizeof(ey_zlib_private_t));
	ret->memory_handler = *mm;
	ret->create_arg = arg;

	sp = &ret->zstream;
	sp->zalloc = ey_zlib_alloc;
	sp->zfree = ey_zlib_free;
	sp->opaque = ret;
	if(ey_zlib_zstream_init(sp, fmt))
	{
		zlib_debug(debug_zlib_basic, "init zstream for fmt %d failed\n", fmt);
		sp = NULL;
		goto failed;
	}
	ret->format = fmt;

	i_buf = (char*)mm->malloc(ilen);
	if(!i_buf)
	{
		zlib_debug(debug_zlib_basic, "alloc input buffer failed\n");
		goto failed;
	}
	ret->input_length = ilen;
	sp->next_in = i_buf;
	sp->avail_in = 0;

	o_buf = (char*)mm->malloc(olen);
	if(!o_buf)
	{
		zlib_debug(debug_zlib_basic, "alloc output buffer failed\n");
		goto failed;
	}
	ret->output_length = olen;
	sp->next_out = o_buf;
	sp->avail_out = olen;
	
	/*now we can return successfully*/
	zlib_debug(debug_zlib_basic, "create ey_zlib successfully\n");
	return ret;

failed:
	if(sp)
		ey_zlib_zstream_finit(sp, fmt);

	if(o_buf)
		mm->free(o_buf);

	if(i_buf)
		mm->free(i_buf);

	if(ret)
		ey_zfree(zlib_slab, ret);
	
	return NULL;
}

void ey_zlib_destroy(ey_zlib_t z)
{
	if(!z)
		return;
	
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)z;
	memory_handler_t *mm = &priv_data->memory_handler;
	z_stream *sp = &priv_data->zstream;
	ey_zlib_format_t fmt = priv_data->format;
	char *i_buf = sp->next_in;
	char *o_buf = sp->next_out;

	ey_zlib_zstream_finit(sp, fmt);
	mm->free(o_buf);
	mm->free(i_buf);
	ey_zfree(zlib_slab, priv_data);
}

const char *ey_zlib_errstr(ey_zlib_t z)
{
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)z;
	
	if(!priv_data)
		return "ey_zlib create failed\n";
	if(!priv_data->err_msg)
		return "no error found\n";
	return priv_data->err_msg;
}

int ey_zlib_pack(ey_zlib_t z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	/*TODO:*/
	return 0;
}

int ey_zlib_unpack(ey_zlib_t z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	/*TODO:*/
	return 0;
}
