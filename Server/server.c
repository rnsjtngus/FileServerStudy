#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <string.h>
#include <fcntl.h> /* for the open options */
#include <unistd.h>
#include <dirent.h>
#include "../Common/common.h"

#define BUFF_MAX_SIZE 1024
#define PORT 7777
#define SAVE_DIR "./Files/"

void set_server_socket(int *server_socket, struct sockaddr_in *server_addr);
void put(int client_socket, MsgHeader h);
void reply_put(int client_socket);
char* get(int clinet_socket, MsgHeader h);
void reply_get(int client_socket, char* file_name);
void list(int client_socket);
void reply_list(int client_socket);

int 
main(char *args, char *argv[]) 
{
	/* 변수 선언 */
	int server_socket;
	int client_socket;
	int client_addr_size;
	int fd;

	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	struct dirent *dir; /* include inode, offset, length, file_name */

	MsgHeader 		header;
	//MsgPUT 			msg_put;
	MsgGET 			msg_get;
	MsgLIST 		msg_list;
	MsgPUTREPLY 	msg_put_reply;
	MsgGETREPLY 	msg_get_reply;
	MsgLISTREPLY 	msg_list_reply;

	DIR *d; /* DIR : directory stream */

	char *file_name;

	/* 서버 소켓 생성 부터 listen 상태까지 세팅 */
	set_server_socket(&server_socket, &server_addr);

	while(1) {
		/* client가 connect시 accept */
		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, 
							(struct sockaddr*) &client_addr, 
							&client_addr_size);

		if(client_socket == -1) {
			printf("Fail to connect with client\n");
			exit(1);
		}
		recv_header(client_socket, &header);

		switch(header.msg_type) {
			case PUT:
			{
				/* read put data and write */
				put(client_socket, header); 

				/* send put_reply */
				reply_put(client_socket);

				break;
			}
			case GET:
				file_name = get(client_socket, header);
				reply_get(client_socket, file_name);
				free(file_name);
				break;
			case LIST:
				list(client_socket, header);
				reply_list(client_socket);
				break;
			default:
				printf("Received wrong message.\n");
				break;
		}
		close(client_socket);
	}
	return 0;
}

/* 서버 소켓 생성 및 설정 */
void 
set_server_socket(int *server_socket, struct sockaddr_in *server_addr) 
{
	/* create socket */
	*server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(*server_socket == -1) {
		printf("Fail to make server socket\n");
		exit(1); 
	}
	printf("Success to make socket\n");

	/* 서버 정보 설정 */
	memset(&(*server_addr), 0, sizeof(*server_addr));
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(PORT);
	server_addr->sin_addr.s_addr = htonl(INADDR_ANY);

	/* 소켓 bind > 소켓에 정보 할당 및 커널 등록 */
	if(bind(*server_socket, (struct sockaddr*) &(*server_addr), sizeof(*server_addr)) == -1) {
		printf("Fail to bind socket\n");
		exit(1);
	}
	printf("Success to bind\n");

	/* 클라이언트 접속 listen */
	if(listen(*server_socket, 1) == -1) {
		printf("Fail to listen\n");
		exit(1);
	}
	printf("Start to Listen\n");
}

void 
put(int client_socket, MsgHeader h) 
{
	/* 변수 선언 */
	int fd;
	MsgPUT *msg_put;
	uint32_t rest;
	MsgHeader header;

	memcpy(&header, &h, sizeof(header));

	recv_put(client_socket, header, msg_put);

	/* start to write file */
	printf("%s\n", msg_put->file_name);
	fd = open(msg_put->file_name, O_WRONLY |O_CREAT);
	write(fd, msg_put->data, (msg_put->header).data_size);	
	rest = (msg_put->header).file_size - (msg_put->header).data_size;
	free_put(msg_put);
	
	while(rest > 0) {
		printf("rest > %zu\n", rest);
		recv_header(client_socket, &header);
		recv_put(client_socket, header, msg_put);
		write(fd, msg_put->data, (msg_put->header).data_size);
		rest = rest - (msg_put->header).data_size;
		free_put(msg_put);
	}
}

void 
reply_put(int client_socket) 
{
	/* 변수 선언 */
	MsgPUTREPLY *msg_put_reply;
	MsgHeader header;
	
	msg_put_reply = malloc(sizeof(MsgPUTREPLY));
	memset(&header, 0, sizeof(MsgHeader));
	
	header.msg_type = PUT_REPLY;
	header.err_code = SUCCESS;
	memcpy(&(msg_put_reply->header), &header, sizeof(MsgHeader));

	send_put_reply(client_socket, msg_put_reply);
	free_put_reply(msg_put_reply);
}

char*
get(int client_socket, MsgHeader header)
{
	MsgGET *msg_get;
	char* file_name;

	recv_get(client_socket, header, msg_get);

	file_name = (char *)malloc(sizeof(char) * header.file_name_size);
	memcpy(file_name, msg_get->file_name, msg_get->header.file_name_size);

	free(msg_get);
	return file_name;	
}

void
reply_get(int client_socket, char *file_name) 
{
	MsgHeader header;
	MsgGETREPLY *msg_get_reply;
	
	int fd, file_size;
	int total_offset;
	int rest, i;

	if((fd = open(file_name, O_RDONLY)) == -1) {
		printf("Cant open file > %s\n", file_name);
		return;
	}

	file_size = get_file_size(file_name);
	total_offset = cceil(((double) file_size) / BUFF_SIZE);
	
	memset(&header, 0, sizeof(header));
	header.file_name_size = strlen(file_name);
	header.owner_size = 0;
	header.total_offset = total_offset;
	header.file_size = file_size;

	rest = file_size;

	for(i = 0; i < total_offset; i++) {
		header.offset = i;
		header.data_size = (rest > BUFF_SIZE) ? BUFF_SIZE : rest;

		rest = (rest > BUFF_SIZE) ? rest - BUFF_SIZE : 0;

		send_get_reply(client_socket, fd, file_name, header, msg_get_reply);
		free_get_reply(msg_get_reply);
	}
	
	close(fd);
}

void
list(int client_socket, MsgHeader header)
{
	MsgLIST *msg_list;
	
	recv_list(client_socket, header, msg_list);
	free(msg_list);	
}

void 
reply_list(int client_socket)
{
	MsgHeader header;
	MsgLISTREPLY *msg_list_reply;

	struct dirent *de;
	
	DIR *dr = opendir(".");

	int offset, total_offset = 0;


	if(dr == NULL) {
		printf("Could not open current directory.\n");
		return;
	}

	memset(&header, 0, sizeof(MsgHeader));
	
	while((de = readdir(dr)) != NULL) {
		total_offset = total_offset + 1;
	}
	closedir(dr);

	header.total_offset = total_offset;	
	offset = 0;

	while((de = readdir(dr)) != NULL) {
		header.data_size = strlen(de->d_name);
		header.offset = offset;

		msg_list_reply = malloc(sizeof(MsgLISTREPLY));
		msg_list_reply->data = (char *)malloc(sizeof(char) * strlen(de->d_name));

		memcpy(&(msg_list_reply->header), &header, sizeof(MsgHeader));
		memcpy(msg_list_reply->data, de->d_name, sizeof(char) * strlen(de->d_name));

		send(client_socket, &(msg_list_reply->header), sizeof(MsgHeader), 0);
		send(client_socket, msg_list_reply->data, sizeof(char) * strlen(de->de_name));

		free(msg_list_reply->data);
		free(msg_list_reply);

		offset = offset + 1;
	}

	closedir(dr);

	return;
}


