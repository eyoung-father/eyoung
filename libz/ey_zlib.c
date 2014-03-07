#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "ey_memory.h"
#include "ey_zlib.h"
#include "zlib.h"
#include "zutil.h"

/* gzip flags */
#define ASCII_FLAG	0x01
#define HEAD_CRC	0x02
#define EXTRA_FIELD	0x04
#define ORIG_NAME	0x08
#define COMMENT		0x10
#define	RESERVED	0xE0

#define MIN_GZ_HEADER_SZ	(10)

enum ey_zlib_gzip_state {
	Z_STATE_NULL,
	Z_STATE_HEADER,
	Z_STATE_EXTRA_FIELD,
	Z_STATE_EXTRA_FIELD_STP2,
	Z_STATE_FNAME,
	Z_STATE_COMMENT,
	Z_STATE_HEAD_CRC,
	Z_STATE_BODY
};

typedef struct ey_zlib_gzip_private
{
	int state;
	int read_len;
	char flags;
	char deflate_try_head;
	unsigned short crc_len;
	unsigned short extra_len;
	char header[MIN_GZ_HEADER_SZ];
	char extra_field[2];
}ey_zlib_gzip_private_t;

typedef struct ey_zlib_private
{
	memory_handler_t memory_handler;
	ey_zlib_gzip_private_t gzip;
	ey_zlib_format_t format;
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

static void *do_zlib_alloc(void *arg, unsigned int n, unsigned int size)
{
	assert(arg != NULL);
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)arg;
	size_t total_len = n * size;
	return priv_data->memory_handler.malloc(total_len);
}

static void do_zlib_free(void *arg, void *ptr)
{
	assert(arg != NULL);
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)arg;
	if(ptr)
		priv_data->memory_handler.free(ptr);
}

static int do_zlib_zstream_init(z_stream *sp, ey_zlib_format_t fmt)
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
			if(inflateInit2(sp, -MAX_WBITS) != Z_OK)
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

static void do_zlib_zstream_finit(z_stream *sp, ey_zlib_format_t fmt)
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

ey_zlib_t ey_zlib_create(memory_handler_t *mm, ey_zlib_format_t fmt, void *arg)
{
	memory_handler_t mm_handler = {ey_malloc,ey_realloc,ey_free,ey_calloc};
	ey_zlib_private_t *ret = NULL;
	z_stream *sp = NULL;
	
	if(!mm)
		mm = &mm_handler;
	
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
	sp->zalloc = do_zlib_alloc;
	sp->zfree = do_zlib_free;
	sp->opaque = ret;
	if(do_zlib_zstream_init(sp, fmt))
	{
		zlib_debug(debug_zlib_basic, "init zstream for fmt %d failed\n", fmt);
		sp = NULL;
		goto failed;
	}
	ret->format = fmt;

	/*now we can return successfully*/
	zlib_debug(debug_zlib_basic, "create ey_zlib successfully\n");
	return ret;

failed:
	if(sp)
		do_zlib_zstream_finit(sp, fmt);

	if(ret)
		ey_zfree(zlib_slab, ret);
	
	return NULL;
}

void ey_zlib_destroy(ey_zlib_t z)
{
	if(!z)
		return;
	
	ey_zlib_private_t *priv_data = (ey_zlib_private_t*)z;
	z_stream *sp = &priv_data->zstream;
	ey_zlib_format_t fmt = priv_data->format;

	do_zlib_zstream_finit(sp, fmt);
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

static int do_deflate(ey_zlib_private_t *z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	#define MAX_DEFLATE_OUT_BUFFER 8192
	char out_buffer[MAX_DEFLATE_OUT_BUFFER];
	z_stream *sp = &z->zstream;
	int r, olen;

	sp->avail_in = i_len;
	sp->next_in = i_buf;
	do
	{
		sp->avail_out = MAX_DEFLATE_OUT_BUFFER;
		sp->next_out = out_buffer;
		r = deflate(sp, i_len ? Z_NO_FLUSH : Z_FINISH);
		if(r != Z_STREAM_END && r != Z_OK)
		{
			z->err_msg = sp->msg;
			zlib_debug(debug_zlib_basic, "deflate return %d\n", r);
			return -1;
		}

		olen = MAX_DEFLATE_OUT_BUFFER - sp->avail_out;
		if(olen)
		{
			r = cb(out_buffer, olen, arg);
			zlib_debug(debug_zlib_basic, "deflate callback return %d\n", r);
			if(r != 0)
				return -1;
		}
	}while(sp->avail_out == 0);

	return 0;
}
static int do_gzip(ey_zlib_private_t *z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	/*TODO:*/
	return 0;
}

int ey_zlib_stream_pack(ey_zlib_t z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	if(!z || !cb || !i_buf || !i_len)
	{
		zlib_debug(debug_zlib_basic, "bad parameter in ey_zlib_stream_pack\n");
		return -1;
	}
	
	ey_zlib_private_t *zp = (ey_zlib_private_t*)z;
	ey_zlib_format_t format = zp->format;
	zp->err_msg = NULL;
	if(format == EY_ZLIB_FORMAT_DEFLATE_PACK)
	{
		if(do_deflate(zp, i_buf, i_len, cb, arg))
		{
			zlib_debug(debug_zlib_basic, "find error in deflate: %s\n", zp->err_msg?zp->err_msg:"unkown error");
			return -1;
		}
		zlib_debug(debug_zlib_basic, "deflate %d bytes successfully\n", i_len);
	}
	else if(format == EY_ZLIB_FORMAT_GZIP_PACK)
	{
		if(do_gzip(zp, i_buf, i_len, cb, arg))
		{
			zlib_debug(debug_zlib_basic, "find error in gzip: %s\n", zp->err_msg?zp->err_msg:"unkown error");
			return -1;
		}
		zlib_debug(debug_zlib_basic, "gzip %d bytes successfully\n", i_len);
	}
	else
	{
		assert(0);
	}
	return 0;
}

#define MIN(x,y) ((x)<(y)?(x):(y))

static int do_gunzip(ey_zlib_private_t *z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	#define MAX_INFLATE_OUT_BUFFER 8192
	char out_buffer[MAX_INFLATE_OUT_BUFFER];
	z_stream *sp = &z->zstream;
	ey_zlib_gzip_private_t *gzip = &z->gzip;
	int r, olen;

	for(;;) {
		switch (gzip->state) {
			case Z_STATE_NULL:
				gzip->state = Z_STATE_HEADER;
				break;
			case Z_STATE_HEADER:
			{
				int read_len = gzip->read_len;
				int cp_len = MIN((unsigned int)(MIN_GZ_HEADER_SZ - read_len), i_len);
				char *header = gzip->header;
				
				memcpy(header + read_len, i_buf, cp_len);

				gzip->read_len += cp_len;
				i_buf += cp_len;
				i_len -= cp_len;

				if (gzip->read_len < MIN_GZ_HEADER_SZ) {
					zlib_debug(debug_zlib_basic, "gzip head collect not complete, read_len: %d", gzip->read_len);
					return 0;
				}

				if (header[0] != '\037' || header[1] != '\213' || header[2] != Z_DEFLATED || (header[3] & RESERVED) != 0) {
					zlib_debug(debug_zlib_basic, "gzip header error: %.2X%.X2%.2X%.2X", header[0], header[1], header[2], header[3]);
					return -1;
				}

				gzip->flags = header[3];
				gzip->state = Z_STATE_EXTRA_FIELD;
				break;
			}
			case Z_STATE_EXTRA_FIELD:
			{
				if (!(gzip->flags&EXTRA_FIELD)) {
					gzip->state = Z_STATE_FNAME;
					break;
				}
				int read_len = gzip->read_len;
				int cp_len = MIN((unsigned int)(2 - (read_len - MIN_GZ_HEADER_SZ)), i_len);
				char *extra_field = gzip->extra_field;
								
				memcpy(extra_field, i_buf, cp_len);

				gzip->read_len += cp_len;
				i_buf += cp_len;
				i_len -= cp_len;

				if (gzip->read_len < MIN_GZ_HEADER_SZ + 2) {
					zlib_debug(debug_zlib_basic, "gzip extra field len collect not complete, read_len: %d", gzip->read_len);
					return 0;
				}

				gzip->extra_len = (unsigned short)extra_field[0] + (((unsigned short)extra_field[1]) << 8);
				gzip->state = Z_STATE_EXTRA_FIELD_STP2;
				break;
			}
			case Z_STATE_EXTRA_FIELD_STP2:
			{
				int skip_len = MIN(gzip->extra_len, i_len);
				i_buf += skip_len;
				i_len -= skip_len;
				gzip->read_len += skip_len;
				gzip->extra_len -= skip_len;

				if (gzip->extra_len > 0) {
					zlib_debug(debug_zlib_basic, "gzip skip extra field len not complete, read_len %d, extra_len: %d", gzip->read_len, gzip->extra_len);
					return 0;
				}

				gzip->state = Z_STATE_FNAME;
				break;
			}
			case Z_STATE_FNAME:
			{
				if (!(gzip->flags&ORIG_NAME)) {
					gzip->state = Z_STATE_COMMENT;
					break;
				}

				char *pos = (char *)memchr(i_buf, '\0', i_len);
				if (!pos) {
					gzip->read_len += i_len;
					zlib_debug(debug_zlib_basic, "gzip skip fname field not complete, read_len %d", gzip->read_len);
					return 0;
				}

				int skip_len = pos - i_buf + 1;

				gzip->read_len += skip_len;
				i_buf += skip_len;
				i_len -= skip_len;
				
				gzip->state = Z_STATE_COMMENT;
				break;
			}
			case Z_STATE_COMMENT:
			{
				if (!(gzip->flags&COMMENT)) {
					gzip->state = Z_STATE_HEAD_CRC;
					break;
				}

				char *pos = (char *)memchr(i_buf, '\0', i_len);
				if (!pos) {
					gzip->read_len += i_len;
					zlib_debug(debug_zlib_basic, "gzip skip comment field not complete, read_len %d", gzip->read_len);
					return 0;
				}

				int skip_len = pos - i_buf + 1;

				gzip->read_len += skip_len;
				i_buf += skip_len;
				i_len -= skip_len;
				
				gzip->state = Z_STATE_HEAD_CRC;
				break;
			}
			case Z_STATE_HEAD_CRC:
			{
				if (!(gzip->flags&HEAD_CRC)) {
					gzip->state = Z_STATE_BODY;
					break;
				}
				
				int skip_len = MIN((unsigned int)(2 - gzip->crc_len), i_len);

				if (gzip->crc_len < 2) {
					gzip->read_len += i_len;
					gzip->crc_len += i_len;
					zlib_debug(debug_zlib_basic, "gzip skip crc field not complete, read_len %d, crc_len", gzip->read_len, gzip->crc_len);
				}

				gzip->read_len += skip_len;
				i_buf += skip_len;
				i_len -= skip_len;

				gzip->state = Z_STATE_BODY;
				break;
			}
			case Z_STATE_BODY:
			{
			    sp->avail_in = i_len;
				sp->next_in = i_buf; 
				do
				{   
			        sp->avail_out = MAX_INFLATE_OUT_BUFFER;
			        sp->next_out = out_buffer;
			        r = inflate(sp, Z_NO_FLUSH);
			        if(r != Z_STREAM_END && r != Z_OK)
					{  
						/* Some server doesn't generate the Zlib header, 
						so here we need make a pesudo header and uncompress again */
						if (r == Z_DATA_ERROR && gzip->deflate_try_head == 0) {
							char pesudo_zlib_header[2] = {0x08 + 0x07 * 0x10, (((0x08 + 0x07 * 0x10) * 0x0100 + 30)/31 *31) & 0xff};
							inflateReset(sp);
							sp->next_in = pesudo_zlib_header;
							sp->avail_in = 2;
							sp->next_out = out_buffer;
							sp->avail_out = MAX_INFLATE_OUT_BUFFER;
							
							r = inflate(sp, Z_NO_FLUSH);

							if (r == Z_OK) {
								sp->avail_in = i_len;
								sp->next_in = i_buf;
								sp->next_out = out_buffer;
								sp->avail_out = MAX_INFLATE_OUT_BUFFER;
								
								r = inflate(sp, Z_NO_FLUSH);
								
								if (r != Z_STREAM_END && r != Z_OK) {
									z->err_msg = sp->msg;
									zlib_debug(debug_zlib_basic, "gunzip try pesudo zlib header and unpack error, return %d\n", r);
									return -1;
								}
							} else {
								z->err_msg = sp->msg;
								zlib_debug(debug_zlib_basic, "gunzip try pesudo zlib header error, return %d\n", r);
								return -1;
							}						
						} else {
							z->err_msg = sp->msg;
							zlib_debug(debug_zlib_basic, "gunzip unpack error, return %d\n", r); 
							return -1;
						}
					}   

			 		olen = MAX_INFLATE_OUT_BUFFER - sp->avail_out;
					
					if(olen)
					{   
			           r = cb(out_buffer, olen, arg);
			           zlib_debug(debug_zlib_basic, "gunzip callback return %d\n", r); 
			           if(r != 0)
			               return -1;
					}   
			   }while(sp->avail_out == 0);	

				break;
			}
			default:
				return -1;
		}
	}
		
	return 0;
}

static int do_inflate(ey_zlib_private_t *z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	#define MAX_INFLATE_OUT_BUFFER 8192
	char out_buffer[MAX_INFLATE_OUT_BUFFER];
	z_stream *sp = &z->zstream;
	int r, olen;

	sp->avail_in = i_len;
	sp->next_in = i_buf;
	do
	{
		sp->avail_out = MAX_INFLATE_OUT_BUFFER;
		sp->next_out = out_buffer;
		r = inflate(sp, Z_NO_FLUSH);
		if(r != Z_STREAM_END && r != Z_OK)
		{
			z->err_msg = sp->msg;
			zlib_debug(debug_zlib_basic, "inflate return %d\n", r);
			return -1;
		}

		olen = MAX_INFLATE_OUT_BUFFER - sp->avail_out;
		if(olen)
		{
			r = cb(out_buffer, olen, arg);
			zlib_debug(debug_zlib_basic, "inflate callback return %d\n", r);
			if(r != 0)
				return -1;
		}
	}while(sp->avail_out == 0);

	return 0;
}

int ey_zlib_stream_unpack(ey_zlib_t z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg)
{
	if(!z || !cb || !i_buf || !i_len)
	{
		zlib_debug(debug_zlib_basic, "bad parameter in ey_zlib_stream_unpack\n");
		return -1;
	}
	
	ey_zlib_private_t *zp = (ey_zlib_private_t*)z;
	ey_zlib_format_t format = zp->format;
	zp->err_msg = NULL;
	if(format == EY_ZLIB_FORMAT_DEFLATE_UNPACK)
	{
		if(do_inflate(zp, i_buf, i_len, cb, arg))
		{
			zlib_debug(debug_zlib_basic, "find error in inflate: %s\n", zp->err_msg?zp->err_msg:"unkown error");
			return -1;
		}
		zlib_debug(debug_zlib_basic, "inflate %d bytes successfully\n", i_len);
	}
	else if(format == EY_ZLIB_FORMAT_GZIP_UNPACK)
	{
		if(do_gunzip(zp, i_buf, i_len, cb, arg))
		{
			zlib_debug(debug_zlib_basic, "find error in gunzip: %s\n", zp->err_msg?zp->err_msg:"unkown error");
			return -1;
		}
		zlib_debug(debug_zlib_basic, "gunzip %d bytes successfully\n", i_len);
	}
	else
	{
		assert(0);
	}
	return 0;
}
