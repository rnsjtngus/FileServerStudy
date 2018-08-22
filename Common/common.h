#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> /* 크기별로 정수 자료형이 정의된 헤더파일 */
#include <sys/stat.h>

#define BUFF_MAX_SIZE 1024

typedef enum _MsgType {
	PUT,
	GET,
	LIST,
	PUT_REPLY,
	GET_REPLY,
	LIST_REPLY,
}MsgType;

typedef enum _ErrorCode{
	NORMAL,
}ErrorCode;

typedef struct _MsgHeader {
	MsgType msg_type;
	ErrorCode err_code;
	uint32_t file_name_size;
	uint32_t owner_size;
	uint32_t total_offset;
	uint32_t curr_offset;
	uint32_t file_size;
	uint32_t data_size;
}MsgHeader;

typedef struct _MsgPUT {
	MsgHeader header;
	char *file_name;
	char *owner;
	char *data;
}MsgPUT;

typedef struct _MsgGET {
	MsgHeader header;
	char *file_name;
	char *owner;
}MsgGET;

typedef struct _MsgLIST {
	MsgHeader header;
	char *owner;
}MsgLIST;

typedef struct _MsgPUTREPLY {
	MsgHeader header;
}MsgPUTREPLY;

typedef struct _MsgGETREPLY {
	MsgHeader header;
	char *file_name;
	char *owner;
	char *buffer;
}MsgGETREPLY;

typedef struct _MsgLISTREPLY {
	MsgHeader header;
	char *file_name;
}MsgLISTREPLY;

void recv_header(int client_socket, MsgHeader header);
void recv_put(int client_socket, MsgHeader header, MsgPUT *msg_put);
void free_put(MsgPUT *msg_put);

/* 주석은 무조건 이거 */
