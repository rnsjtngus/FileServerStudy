#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum _msg_type_t {
	MSG_PUT,
	MSG_GET,
	MSG_LIST,
	MSG_PUT_REPLY,
	MSG_GET_REPLY,
	MSG_LIST_REPLY,
} msg_type_t;

typedef struct _msg_meta_t {
	msg_type_t 	type;
	uint32_t 	file_name_size;
	uint32_t 	owner_name_size;
	uint32_t 	total_offset;
	uint32_t 	max_buff_size;
	uint32_t 	file_size;
	char* 		file_name;
	char* 		owner_name;
} msg_meta_t;

typedef struct _msg_put_t {
	uint32_t 	curr_offset;
	uint32_t 	buff_size;
	char* 		buff;
} msg_put_t;

typedef struct _msg_get_t {
	
} msg_get_t;

typedef struct _msg_list_t {

} msg_list_t;

typedef struct _msg_put_reply_t { 

} msg_put_reply_t;

typedef struct _msg_get_reply_t {
	uint32_t 	curr_offset;
	uint32_t	buff_size;
	char* 		buff;
} msg_get_reply_t;

typedef struct _msg_list_reply_t {
	uint32_t 	curr_offset;
	uint32_t	buff_size;
	char* 		buff;
} msg_list_reply_t;

/* functions in message.c */
void msg_send(int socket_fd, const void *msg, size_t len, int flags);
void msg_recv(int socket_fd, void *buff, size_t len, int flags);
void msg_meta_recv(int socket_fd, msg_meta_t *msg_meta);
void msg_meta_free(msg_meta_t *msg_meta);
