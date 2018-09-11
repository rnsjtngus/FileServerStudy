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
#include "../Common/message.h"

#define MAX_USER_NAME_SIZE 1024
#define MAX_FILE_NAME_SIZE 1024
#define MAX_BUFF_SIZE 1024

#define REPLY_MSG_SIZE 1024
#define FILE_NAME_SIZE 1024
#define BUFF_SIZE 1024
#define PORT 7777

void set_client_socket(int *client_socekt, struct sockaddr_in *server_addr); 
void client_put(int client_socket);
void client_reply_put(int client_socket);
void client_get(int client_socket);
void client_reply_get(int client_socket);
void client_list(int client_socket);
void client_reply_list(int client_socket);

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
				client_put(client_socket);
				client_reply_put(client_socket);
				break;
			case 2: // GET
				client_get(client_socket);
				client_reply_get(client_socket);
				break;
			case 3: // LIST
				client_list(client_socket);
				client_reply_list(client_socket);
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
client_put(int client_socket)
{
	int fd, total_offset, i;
	uint32_t file_size, rest;	
	char user_name[MAX_USER_NAME_SIZE];
	char file_name[MAX_FILE_NAME_SIZE];
	char buff[MAX_BUFF_SIZE];
	msg_meta_t *msg_meta; 
	msg_put_t *msg_put;

	/* get file name from user */
	printf("Enter the file name > ");
	scanf("%s", file_name);

	if((fd = open(file_name, O_RDONLY, S_IRWXU)) == -1) {
		printf("[PUT] Fail to open file. Try again.\n");
		return;
	}

	/* get file info */
	file_size = get_file_size(file_name);
	total_offset = cceil(((double) file_size) / MAX_BUFF_SIZE);

	/* get user info */
	getlogin_r(user_name, MAX_USER_NAME_SIZE);

	/* set msg meta */
	msg_meta = (msg_meta_t *)malloc(sizeof(msg_meta_t));
	msg_meta->file_name = (char *)malloc(sizeof(char) * strlen(file_name));
	msg_meta->owner_name = (char *)malloc(sizeof(char) * strlen(user_name));

	msg_meta->type = MSG_PUT;
	msg_meta->file_name_size = strlen(file_name);
	msg_meta->owner_name_size = strlen(user_name);
	msg_meta->total_offset = total_offset;
	msg_meta->max_buff_size = MAX_BUFF_SIZE;
	msg_meta->file_size = file_size;

	memcpy(msg_meta->file_name, file_name, strlen(file_name));
	memcpy(msg_meta->owner_name, user_name, strlen(user_name));

	/* send msg meta */
	msg_send(client_socket, msg_meta, sizeof(msg_meta_t) - sizeof(char *) * 2, 0);
	msg_send(client_socket, msg_meta->file_name, msg_meta->file_name_size, 0);
	msg_send(client_socket, msg_meta->owner_name, msg_meta->owner_name_size, 0);

	/* free msg meta */
	msg_meta_free(msg_meta);

	rest = file_size;
	for(i = 0; i++; i < total_offset) {
		/* set msg put */
		msg_put = (msg_put_t *)malloc(sizeof(msg_put_t));
		msg_put->curr_offset = i;
		msg_put->buff_size = (rest > MAX_BUFF_SIZE) ? MAX_BUFF_SIZE : rest;
		rest = (rest > MAX_BUFF_SIZE) ? rest - MAX_BUFF_SIZE : 0;
		msg_put->buff = (char *)malloc(sizeof(char) * msg_put->buff_size);
		read(fd, msg_put->buff, sizeof(char) * msg_put->buff_size);

		/* send msg put */
		msg_send(client_socket, msg_put, sizeof(msg_put_t) - sizeof(char *), 0);
		msg_send(client_socket, msg_put->buff, sizeof(char) * msg_put->buff_size, 0);

		/* free msg put */
		free(msg_put->buff);
		free(msg_put);
	}
	close(fd);
	return;
}

void
client_reply_put(int client_socket)
{
	msg_meta_t *msg_meta;

	/* recv msg meta */
	msg_meta_recv(client_socket, msg_meta);

	/* check success or fail */
	printf("[PUT] SUCCESS\n");

	/* free msg meta */
	free(msg_meta);

	return;
}

void
client_get(int client_socket)
{
	char file_name[MAX_FILE_NAME_SIZE];
	char owner_name[MAX_USER_NAME_SIZE];
	msg_meta_t *msg_meta;
	msg_get_t *msg_get;

	/* get file name from user */
	printf("Enter the file name > ");
	scanf("%s", file_name);
	
	/* get user name */
	getlogin_r(owner_name, MAX_USER_NAME_SIZE);

	/* init msg meta */
	msg_meta = (msg_meta_t *)malloc(sizeof(msg_meta_t));
	msg_meta->file_name = (char *)malloc(sizeof(char) * strlen(file_name));
	msg_meta->owner_name = (char *)malloc(sizeof(char) * strlen(owner_name));

	msg_meta->type = MSG_GET;
	msg_meta->file_name_size = strlen(file_name);
	msg_meta->owner_name_size = strlen(owner_name);
	msg_meta->total_offset = 0;
	msg_meta->max_buff_size = 0;
	msg_meta->file_size = 0;

	memcpy(msg_meta->file_name, file_name, sizeof(char) * strlen(file_name));
	memcpy(msg_meta->owner_name, owner_name, sizeof(char) * strlen(owner_name));

	/* send msg meta */
	msg_send(client_socket, msg_meta, sizeof(msg_meta) - sizeof(char *) * 2, 0);
	msg_send(client_socket, msg_meta->file_name, sizeof(char) * msg_meta->file_name_size, 0);
	msg_send(client_socket, msg_meta->owner_name, sizeof(char) * msg_meta->owner_name_size, 0);

	/* free msg meta */
	msg_meta_free(msg_meta);

	return;
}

void
client_reply_get(int client_socket)
{
	int fd, i, curr_offset;
	msg_meta_t *msg_meta;
	msg_get_reply_t *msg_get_reply;

	/* init msg meta & receive msg meta */
	msg_meta_recv(client_socket, msg_meta);

	/* ready for write file */
	if((fd = open(msg_meta->file_name, O_WRONLY | O_CREAT, S_IRWXU)) == -1) {
		printf("[GET] Fail to open file. Try again.\n");
		return;
	}

	/* start to write file */
	for(i = 0; i++; i < msg_meta->total_offset) {
		/* init & recv msg get reply */
		msg_get_reply = (msg_get_reply_t *)malloc(sizeof(msg_get_reply_t));
		msg_recv(client_socket, msg_get_reply, sizeof(msg_get_reply_t) - sizeof(char *), 0);
		msg_get_reply->buff = (char *)malloc(sizeof(char) * msg_get_reply->buff_size);
		msg_recv(client_socket, msg_get_reply->buff, sizeof(char) * msg_get_reply->buff_size, 0);
		
		/* write file */
		curr_offset = msg_get_reply->curr_offset * MAX_BUFF_SIZE;
		pwrite(fd, msg_get_reply->buff, msg_get_reply->buff_size * sizeof(char), curr_offset);

		/* free msg get reply */
		free(msg_get_reply->buff);
		free(msg_get_reply);
	}
	/* free fd */
	close(fd);

	/* free msg meta */
	msg_meta_free(msg_meta);

	return;
}

void
client_list(int client_socket)
{
	msg_meta_t *msg_meta;
	char user_name[MAX_USER_NAME_SIZE];

	/* get user name */
	getlogin_r(user_name, MAX_USER_NAME_SIZE);

	/* init msg meta */
	msg_meta = (msg_meta_t *)malloc(sizeof(msg_meta_t));
	msg_meta->type = MSG_LIST;
	msg_meta->owner_name_size = strlen(user_name);
	msg_meta->file_name_size = 0;
	
	msg_meta->file_name = (char *)malloc(sizeof(char) * 0);
	msg_meta->owner_name = (char *)malloc(sizeof(char) * strlen(user_name));

	memcpy(msg_meta->owner_name, user_name, strlen(user_name));
	
	/* send msg meta */
	msg_send(client_socket, msg_meta, sizeof(msg_meta_t) - sizeof(char *) * 2, 0);
	msg_send(client_socket, msg_meta->owner_name, sizeof(char) * strlen(user_name), 0);

	/* free msg meta */
	msg_meta_free(msg_meta);

	return;
}

void
client_reply_list(int client_socket)
{
	int file_num, i;
	msg_meta_t *msg_meta;
	msg_list_reply_t *msg_list_reply;

	/* recv msg meta */
	msg_meta_recv(client_socket, msg_meta);
	
	/* recv msg list reply */
	file_num = msg_meta->total_offset;

	printf("[ %s's files ]\n", msg_meta->owner_name);
	for(i = 0; i < file_num; i++) {
		/* init & recv msg list reply */
		msg_list_reply = (msg_list_reply_t *)malloc(sizeof(msg_list_reply_t));
		msg_recv(client_socket, msg_list_reply, sizeof(msg_list_reply_t) - sizeof(char *), 0);
		
		msg_list_reply->buff = (char *)malloc(sizeof(char) * msg_list_reply->buff_size);
		msg_recv(client_socket, msg_list_reply, sizeof(char) * msg_list_reply->buff_size, 0);

		/* print list */
		printf("%2d. %s\n", i + 1, msg_list_reply->buff);

		/* free msg list reply */
		free(msg_list_reply->buff);
		free(msg_list_reply);
	}
	printf("[ End of List ]\n");

	/* free msg meta */
	msg_meta_free(msg_meta);
	
	return;
}
