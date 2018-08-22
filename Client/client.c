#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include "../Common/common.h"

#define REPLY_MSG_SIZE 1024
#define FILE_NAME_SIZE 1024
#define BUFF_SIZE 1024
#define PORT 7777


void set_client_socket(int *client_socekt, struct sockaddr_in *server_addr); 
void put(int client_socket);
void reply_put(int client_socket);
uint32_t get_file_size(char *file_name);

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
				break;
			
			case 3: // LIST
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
	total_offset = ((double) file_size) / BUFF_SIZE ;
	
	memset(&header, 0, sizeof(header));
	header.file_name_size = strlen(file_name);
	header.owner_size = 0;
	header.total_offset = total_offset;
	//header.curr_offset = 0;
	//header.data_size = (header.offset == total_offset - 1) ? file_size : BUFF_SIZE;
	
	rest = file_size;

	for(i = 0; i < total_offset; i++) {
		header.curr_offset = i;
		header.data_size = (rest > BUFF_SIZE) ? BUFF_SIZE : rest;

		rest = (rest > BUFF_SIZE) ? rest - BUFF_SIZE : 0;

		msg_put = malloc(sizeof(MsgPUT));
		msg_put->file_name = malloc(sizeof(char) * header.file_name_size);
		msg_put->owner = malloc(sizeof(char) * header.owner_size);
		msg_put->data = malloc(sizeof(char) * header.data_size);
		
		memcpy(&(msg_put->header), &header, sizeof(header));
		memcpy(msg_put->file_name, file_name, strlen(file_name));
		read(fd, msg_put->data, header.data_size);

		send(client_socket, &(msg_put->header), sizeof(header), 0);
		send(client_socket, msg_put->file_name, header.file_name_size, 0);
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

}

uint32_t 
get_file_size(char *file_name) 
{
	struct stat st;
	stat(file_name, &st);
	uint32_t size = st.st_size;
	return size;
}

