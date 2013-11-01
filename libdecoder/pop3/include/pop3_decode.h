#ifndef POP3_DECODE_H
#define POP3_DECODE_H 1

typedef void* pop3_work_t;

extern void pop3_decoder_init();
extern void pop3_decoder_finit();

extern pop3_work_t pop3_work_create(int greedy);
extern int pop3_decode_data(pop3_work_t work, const char *data, size_t data_len, int from_client, int last_frag);
extern void pop3_work_destroy(pop3_work_t work);
#endif
