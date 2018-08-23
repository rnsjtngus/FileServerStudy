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
void reply_put();

int main(char *args, char *argv[]) {
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
		
		/* recv request header */
		recv_header(client_socket, &header);
		//printf("First header : %zu, %zu, %zu\n", header.file_name_size, header.owner_size,
		//		header.data_size);

		switch(header.msg_type) {
			case PUT:
			{
				/* read put data and write */
				put(client_socket, header); 
				
				/*
				recv(client_socket, file_name, recv_h.body_size, 0);
				printf("Received file name > %s\n", file_name);		
				file_name[recv_h.body_size] = '\0';

				fdw = open(file_name, O_WRONLY | O_CREAT);
				printf("File name > %s\n", file_name);
				int len = (int) recv_h.buff_size;
				while(len > 0) {
					printf("Downloading...\n");
					if(len > BUFF_SIZE) {
						recv(client_socket, buff_rcv, BUFF_SIZE, 0);
						write(fdw, buff_rcv, BUFF_SIZE);
						len = len - BUFF_SIZE;
					} else {
						recv(client_socket, buff_rcv, len, 0);
						write(fdw, buff_rcv, len);
						len = 0;
					}
					printf("Write > %s\n", buff_rcv);
				}
				printf("Download Successfully\n");
				*/

				/* send put_reply */
				reply_put();

				/*
				send_h.body_size = 1;
				send_h.buff_size = 7;
				send_h.msg_type = PUT_REPLY;

				send(client_socket, &send_h, sizeof(send_h), 0);

				send(client_socket, "0", 1, 0);
				send(client_socket, "success", 7, 0);
		
				close(client_socket);
				close(fdw);*/
				break;
			}
			case GET:
				break;
			case LIST:
				break;
			default:
				printf("Received wrong message.\n");
				break;
		}
		close(server_socket);
	}
	return 0;
}

/* 서버 소켓 생성 및 설정 */
void set_server_socket(int *server_socket, struct sockaddr_in *server_addr) {
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

void put(int client_socket, MsgHeader h) {
	/* 변수 선언 */
	int fd;
	MsgPUT *msg_put;
	uint32_t rest;
	MsgHeader header;

	memcpy(&header, &h, sizeof(header));

	printf("A\n");
	recv_put(client_socket, header, msg_put);
	printf("Done recv put\n");
	printf("RECEIVED : %d, %d\n", sizeof(*msg_put), sizeof(MsgPUT));
	printf("RECEIVED : %s\n", &(*msg_put).file_name);
	printf("RECEIVED : %s, %s, %s\n", msg_put->file_name, msg_put->owner, msg_put->data);

	/* start to write file */
	printf("B\n");
	printf("%s\n", msg_put->file_name);
	fd = open(msg_put->file_name, O_WRONLY |O_CREAT);

	printf("C\n");
	write(fd, msg_put->data, (msg_put->header).data_size);	
	rest = (msg_put->header).file_size - (msg_put->header).data_size;

	printf("D\n");
	free_put(msg_put);

	printf("E\n");
	while(rest > 0) {
		printf("F\n");
		recv_header(client_socket, &header);
		recv_put(client_socket, header, msg_put);
		write(fd, msg_put->data, (msg_put->header).data_size);
		rest = rest - (msg_put->header).data_size;
		free_put(msg_put);
	}
	printf("DONEDONEDONE\n");
}

void reply_put() {

}


