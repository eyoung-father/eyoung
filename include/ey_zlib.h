#ifndef EY_ZLIB_H
#define EY_ZLIB_H 1

typedef void* ey_zlib_t;

typedef enum ey_zlib_format
{
	EY_ZLIB_FORMAT_GZIP_PACK,
	EY_ZLIB_FORMAT_GZIP_UNPACK,
	EY_ZLIB_FORMAT_DEFLATE_PACK,
	EY_ZLIB_FORMAT_DEFLATE_UNPACK,

	EY_ZLIB_FORMAT_UNKOWN
}ey_zlib_format_t;

struct memory_handler;
extern ey_zlib_t ey_zlib_create(struct memory_handler *mm, ey_zlib_format_t fmt, void *arg);
extern void ey_zlib_destroy(ey_zlib_t z);

typedef int (*ey_zlib_callback)(ey_zlib_t z, char *o_buf, size_t o_len, void *arg);
extern int ey_zlib_pack(ey_zlib_t z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg);
extern int ey_zlib_unpack(ey_zlib_t z, char *i_buf, size_t i_len, ey_zlib_callback cb, void *arg);
extern const char *ey_zlib_errstr(ey_zlib_t z);
#endif
