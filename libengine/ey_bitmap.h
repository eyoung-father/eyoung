#ifndef EY_BITMAP_H
#define EY_BITMAP_H 1

typedef struct ey_bitmap
{
	unsigned char *bitmap;
	unsigned long size;
	unsigned long count;
}ey_bitmap_t;

extern int ey_bitmap_init();
extern void ey_bitmap_finit();
extern ey_bitmap_t *ey_bitmap_create(unsigned long count);
extern void ey_bitmap_destroy(ey_bitmap_t *bitmap);
extern void ey_bitmap_set(ey_bitmap_t *bitmap, unsigned long nbit);
extern void ey_bitmap_clear(ey_bitmap_t *bitmap, unsigned long nbit);
extern int ey_bitmap_isset(ey_bitmap_t *bitmap, unsigned long nbit);
#endif
