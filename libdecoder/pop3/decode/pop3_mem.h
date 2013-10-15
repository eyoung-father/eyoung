#ifndef POP3_MEM_H
#define POP3_MEM_H 1

/*memory mgt system init api for system initializing*/
extern void pop3_mem_init();
extern void pop3_mem_finit();

/*slab mgt api*/
struct pop3_data;
extern struct pop3_data* pop3_alloc_priv_data(int greedy);
extern void pop3_free_priv_data(struct pop3_data *priv_data);

struct pop3_response;
struct pop3_line;
struct pop3_res_content;
extern struct pop3_response* pop3_alloc_response(int res_code, char *msg, int msg_len, struct pop3_res_content *content);
extern struct pop3_line* pop3_alloc_response_line(char *data, int data_len);
extern void pop3_free_response(struct pop3_response *res);
extern void pop3_free_response_content(struct pop3_res_content *head);

struct pop3_request;
extern struct pop3_request* pop3_alloc_request(int req_code, ...);
extern void pop3_free_request(struct pop3_request *req);

struct pop3_req_arg;
struct pop3_req_arg_list;
extern struct pop3_req_arg* pop3_alloc_req_arg(char *data, int data_len);
extern void pop3_free_req_arg_list(struct pop3_req_arg_list *head);

struct pop3_cmd;
struct pop3_cmd_list;
extern struct pop3_cmd* pop3_alloc_cmd(struct pop3_request *req, struct pop3_response *res);
extern void pop3_free_cmd_list(struct pop3_cmd_list *head);

#include <stdlib.h>
#include "ey_memory.h"

/*KMALLOC/KFREE*/
#define pop3_malloc(sz) ey_malloc(sz)
#define pop3_realloc(ptr,sz) ey_realloc(ptr,sz)
#define pop3_free(p) ey_free(p)

/*for slab*/
#define pop3_zalloc(slab) ey_zalloc(slab)
#define pop3_zfree(slab,p) ey_zfree(slab,p)
#define pop3_zclear(slab) ey_zclear(slab)
#define pop3_zfinit(slab) ey_zfinit(slab)
#define pop3_zinit(name,size) ey_zinit((name),(size),NULL)

#endif
