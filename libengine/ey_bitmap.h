#ifndef EY_BITMAP_H
#define EY_BITMAP_H 1

typedef struct ey_bitmap
{
	unsigned char *bitmap;
	unsigned long size;
	unsigned long count;
}ey_bitmap_t;

struct ey_engine;
extern int ey_bitmap_init(struct ey_engine *eng);
extern void ey_bitmap_finit(struct ey_engine *eng);
extern ey_bitmap_t *ey_bitmap_create(struct ey_engine *eng, unsigned long count);
extern void ey_bitmap_destroy(struct ey_engine *eng, ey_bitmap_t *bitmap);
extern void ey_bitmap_set(struct ey_engine *eng, ey_bitmap_t *bitmap, unsigned long nbit);
extern void ey_bitmap_clear(struct ey_engine *eng, ey_bitmap_t *bitmap, unsigned long nbit);
extern int ey_bitmap_isset(struct ey_engine *eng, ey_bitmap_t *bitmap, unsigned long nbit);
#endif
