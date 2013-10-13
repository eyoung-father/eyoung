#ifndef POP3_PRIVATE_H
#define POP3_PRIVATE_H 1

struct yy_buffer_state;
struct pop3_data;

extern struct yy_buffer_state* pop3_client_scan_stream(const char *new_buf, size_t new_buf_len, struct pop3_data *priv);
extern struct yy_buffer_state* pop3_server_scan_stream(const char *new_buf, size_t new_buf_len, struct pop3_data *priv);

extern int parse_pop3_client_stream(struct pop3_data *priv, const char *buf, size_t buf_len, int last_frag);
extern int parse_pop3_server_stream(struct pop3_data *priv, const char *buf, size_t buf_len, int last_frag);
#endif
