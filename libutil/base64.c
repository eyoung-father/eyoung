#include "libutil.h"

#define B64_MAP_TABLE_INDEX_MIN ('+')
#define B64_MAP_TABLE_INDEX_MAX ('z')
#define B64_MAP_TABLE_LEN       (B64_MAP_TABLE_INDEX_MAX -   \
                                 B64_MAP_TABLE_INDEX_MIN + 1)
#define BERR                    (B64_MAP_TABLE_INDEX_MAX + 1)
#define BEND                    (B64_MAP_TABLE_INDEX_MAX + 2)
static char B64_MAP_TABLE[B64_MAP_TABLE_LEN] = 
{
//  +     ,     -     .     /     0     1     2
    0x3e, BERR, BERR, BERR, 0x3f, 0x34, 0x35, 0x36,
//  3     4     5     6     7     8     9     :
    0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, BERR,
//  ;     <     =     >     ?     @     A     B
    BERR, BERR, BEND, BERR, BERR, BERR, 0x00, 0x01,
//  C     D     E     F     G     H     I     J
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
//  K     L     M     N     O     P     Q     R
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
//  S     T     U     V     W     X     Y     Z
    0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
//  [     \     ]     ^     _     `     a     b
    BERR, BERR, BERR, BERR, BERR, BERR, 0x1a, 0x1b,
//  c     d     e     f     g     h     i     j
    0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23,
//  k     l     m     n     o     p     q     r
    0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b,
//  s     t     u     v     w     x     y     z
    0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33,
};


static inline char _base64_get_index(char src)
{
	if ((B64_MAP_TABLE_INDEX_MIN > src) ||
		(B64_MAP_TABLE_INDEX_MAX < src))
	{
		return BERR;
	}
	return B64_MAP_TABLE[src - B64_MAP_TABLE_INDEX_MIN];
}


int base64_stream_decode(char* dst, char* src, int src_len, int* left)
{
	char * p = dst;
	char * s = src;

	int in_len = 0;
	int out_len = 0;
	int i, k, effect_len=0;
	char prev = 0;
	for (i = 0; i<src_len; i++)
	{
		char v = *(s++);
		k = effect_len % 4;

		char ch = _base64_get_index(v);

		if (BERR == ch)
		{
			if(v=='\r' || v=='\n' ||
				v==' ' || v=='\t')
				continue;
			ch=0;
		}
		else if (BEND == ch)
		{
			*left = src_len - i - 1;
			*p = '\0';
			return p - dst;
		}
		effect_len ++;

		switch (k)
		{
		case 0:
			break;

		case 1:
			*(p++) = ((prev << 2) & 0xfc) |
				((ch >> 4) & 0x03);
			break;

		case 2:
			*(p++) = ((prev << 4) & 0xf0) |
				((ch >> 2) & 0x0f);
			break;

		case 3:
			*(p++) = ((prev << 6) & 0xc0) |
				(ch & 0x03f);
			in_len += 4;
			out_len += 3;
			break;
		}
		prev = ch;
	}
	*p = '\0';
	*left = 0;
	return out_len;
}
