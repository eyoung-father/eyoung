#ifndef POP3_DECODE_H
#define POP3_DECODE_H 1

typedef void* pop3_decode_t;

extern void pop3_decode_init();
extern void pop3_decode_finit();

extern pop3_decode_t pop3_decode_create(int greedy);
extern int pop3_decode_data(pop3_decode_t decoder, const char *data, size_t data_len, int from_client, int last_frag);
extern void pop3_decode_destroy(pop3_decode_t decode);
#endif
