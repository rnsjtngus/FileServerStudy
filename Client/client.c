#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
//#include <math.h>
#include "../Common/common.h"

#define REPLY_MSG_SIZE 1024
#define FILE_NAME_SIZE 1024
#define BUFF_SIZE 1024
#define PORT 7777

void set_client_socket(int *client_socekt, struct sockaddr_in *server_addr); 
void put(int client_socket);
void reply_put(int client_socket);
void get(int client_socket);
void reply_get(int client_socket);
void list(int client_socket);
void reply_list(int client_socket);
uint32_t get_file_size(char *file_name);
int cceil(double x);

int 
main(int argc, char *argv[]) 
{
	// 변수선언
	int client_socket;
	int command;
	int fd;
	int rest;
	//int err_code;

	struct sockaddr_in server_addr;
	
	char buff[BUFF_SIZE];
	char file_name[FILE_NAME_SIZE];
	char err_code;

	size_t file_size;

	MsgHeader header;

	// Ask to user
	while(1) {
		printf("Select (1)PUT, (2)GET, (3)LIST, (4)QUIT.(only integer) > ");
		scanf("%d", &command); 

		/* 클라이언트 소켓 설정 및 서버 연결 */
		set_client_socket(&client_socket, &server_addr);

		switch(command) {
			case 1: // PUT
				put(client_socket);
				reply_put(client_socket);
				break;
			
			case 2: // GET
				get(client_socket);
				reply_get(client_socket);
				break;
			
			case 3: // LIST
				list(client_socket);
				reply_list(client_socket);
				break;
			
			case 4: // QUIT
				printf("Quit.\n");
				close(client_socket);
				exit(1);
				break;
			
			default:
				printf("Wrong Input. Try Again.\n");
				break;
		}

		/* 클라이언트 소켓 닫기 */
		close(client_socket);
	}

	return 0;
}

void 
set_client_socket(int *client_socket, struct sockaddr_in *server_addr) 
{
	// create socket
	*client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(*client_socket == -1) {
		printf("Fail to create client socket.\n");
		exit(1);
	}

	// setting server info
	memset(server_addr, 0, sizeof(*server_addr));
	(*server_addr).sin_family = AF_INET;
	(*server_addr).sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &((*server_addr).sin_addr));

	if(connect(*client_socket, (struct sockaddr*) server_addr, sizeof(*server_addr)) == -1) {
		printf("Fail to connect.\n");
		//printf("%s\n", strerror(errno));
		exit(1);
	}
}

void
put(int client_socket) 
{
	/* 변수 선언 */
	int fd;
	int total_offset;
	int i;

	uint32_t file_size, rest;

	char file_name[FILE_NAME_SIZE];
	char buff[BUFF_SIZE];

	MsgHeader header;

	MsgPUT *msg_put;

	printf("Enter the file name > ");
	scanf("%s", file_name);
				
	if((fd = open(file_name, O_RDONLY)) == -1) {
		printf("[PUT] Fail to open file. Try again.\n");
		return;
	}

	file_size = get_file_size(file_name);
	total_offset = cceil(((double) file_size) / BUFF_SIZE);
	
	memset(&header, 0, sizeof(header));
	header.file_name_size = strlen(file_name);
	header.owner_size = 0;
	header.total_offset = total_offset;
	header.file_size = file_size;
	//header.curr_offset = 0;
	//header.data_size = (header.offset == total_offset - 1) ? file_size : BUFF_SIZE;
	
	rest = file_size;

	for(i = 0; i < total_offset; i++) {
		header.offset = i;
		header.data_size = (rest > BUFF_SIZE) ? BUFF_SIZE : rest;

		rest = (rest > BUFF_SIZE) ? rest - BUFF_SIZE : 0;

		msg_put = (MsgPUT *)malloc(sizeof(MsgPUT));
		msg_put->file_name 	= (char *)malloc(sizeof(char) * header.file_name_size);
		msg_put->owner 		= (char *)malloc(sizeof(char) * header.owner_size);
		msg_put->data 		= (char *)malloc(sizeof(char) * header.data_size);
		
		memcpy(&(msg_put->header), &header, sizeof(header));
		memcpy(msg_put->file_name, file_name, sizeof(char) * header.file_name_size);
		read(fd, msg_put->data, sizeof(char) * header.data_size);

		send(client_socket, &(msg_put->header), sizeof(header), 0);
		send(client_socket, msg_put->file_name, header.file_name_size, 0);
		send(client_socket, msg_put->owner, header.owner_size, 0);
		send(client_socket, msg_put->data, header.data_size, 0);

		free(msg_put->file_name);
		free(msg_put->owner);
		free(msg_put->data);
		free(msg_put);
	}

	close(fd);
}

void
reply_put(int client_socket) 
{
	MsgPUTREPLY *msg_put_reply;
	MsgHeader header;

	msg_put_reply = malloc(sizeof(MsgPUTREPLY));
	memset(&header, 0, sizeof(MsgHeader));

	recv(client_socket, &header, sizeof(MsgHeader), 0);

	memcpy(&(msg_put_reply->header), &header, sizeof(MsgHeader));

	if((msg_put_reply->header).err_code != SUCCESS) {
		printf("[PUT REPLY] Fail.\n");
	} else {
		printf("[PUT REPLY] Success.\n");
	}

	free(msg_put_reply);
}

void
get(int client_socket) 
{
	/* 변수 선언 */
	int file_name_size;
	
	char file_name[FILE_NAME_SIZE];

	MsgHeader header;
	MsgGET *msg_get;

	printf("Enter the file name > ");
	scanf("%s", file_name);
	file_name_size = strlen(file_name);

	memset(&header, 0, sizeof(MsgHeader));

	header.msg_type 		= GET;
	header.file_name_size 	= file_name_size;
	header.owner_size 		= 0;

	msg_get 			= malloc(sizeof(MsgGET));
	msg_get->file_name 	= (char *)malloc(sizeof(char) * header.file_name_size);
	msg_get->owner 		= (char *)malloc(sizeof(char) * header.owner_size);
	
	memcpy(&(msg_get->header), &header, sizeof(MsgHeader));
	memcpy(msg_get->file_name, file_name, file_name_size);
	memcpy(msg_get->owner, 0, 0);

	send(client_socket, &header, sizeof(MsgHeader), 0);
	send(client_socket, msg_get->file_name, file_name_size, 0);

	free(msg_get->owner);
	free(msg_get->file_name);
	free(msg_get);
}

void 
reply_get(int client_socket)
{
	/* 변수 선언 */
	MsgHeader header;
	MsgGETREPLY *msg_get_reply;

	uint32_t rest;
	
	int fd;

	memset(&header, 0, sizeof(MsgHeader));
	
	recv(client_socket, &header, sizeof(MsgHeader), 0);

	recv_get_reply(client_socket, header, msg_get_reply);

	fd = open(msg_get_reply->file_name, O_WRONLY | O_CREAT);
	write(fd, msg_get_reply->data, (msg_get_reply->header).data_size);
	rest = (msg_get_reply->header).file_size - (msg_get_reply->header).data_size;
	free_get_reply(msg_get_reply);

	while(rest > 0) {
		msg_get_reply = malloc(sizeof(MsgGETREPLY));
		recv(client_socket, &header, sizeof(MsgHeader), 0);
		recv_get_reply(client_socket, header, msg_get_reply);
		write(fd, msg_get_reply->data, msg_get_reply->header.data_size);
		rest = rest - (msg_get_reply->header).data_size;
		free_get_reply(msg_get_reply);
	}
}

void
list(int client_socket) 
{
	/* 변수 선언 */
	MsgHeader header;
	MsgLIST *msg_list;

	memset(&header, 0, sizeof(MsgHeader));
	
	header.msg_type = LIST;
	header.owner_size = 0;

	msg_list = malloc(sizeof(MsgLIST));
	msg_list->owner = (char *)malloc(sizeof(char) * header.owner_size);
	
	send(client_socket, &header, sizeof(MsgHeader), 0);
	
	free(msg_list->owner);
	free(msg_list);
}

void 
reply_list(int client_socket)
{
	/* 변수 선언 */
	MsgHeader header;
	MsgLISTREPLY *msg_list_reply;

	int rest;

	memset(&header, 0, sizeof(MsgHeader));
	recv(client_socket, &header, sizeof(MsgHeader), 0);

	recv_list_reply(client_socket, header, msg_list_reply);

	rest = msg_list_reply->header.total_offset;

	printf("=== List of files (Total : %d) ===\n", rest);

	if(rest > 0) {
 		printf("%d. %s\n", msg_list_reply->header.offset, msg_list_reply->file_name);
	}

	free_list_reply(msg_list_reply);

	while(rest > 0) {
		recv(client_socket, &header, sizeof(MsgHeader), 0);
		recv_list_reply(client_socket, header, msg_list_reply);
		printf("%d. %s\n", msg_list_reply->header.offset, msg_list_reply->file_name);
		free_list_reply(msg_list_reply);
	}
	printf("=== End of files list ===\n");
}

/*
uint32_t 
get_file_size(char *file_name) 
{
	struct stat st;
	stat(file_name, &st);
	uint32_t size = st.st_size;
	return size;
}

int 
cceil(double x) 
{
	if((x - (int) x) > 0.0) {
		return (int)x + 1;
	}
	return (int)x;
}
*/
