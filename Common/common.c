// File Description comment

// #include files
#include "common.h"
// Constant, static type definitions


// Global variables and functions exported
//void set_header(MsgHeader *header, size_t body_size, size_t buff_size, MessageType msg_type);

// variables and functions
void send_header(int client_socket, MsgHeader header) {
	send(client_socket, &header, sizeof(header), 0);
}

void recv_header(int client_socket, MsgHeader *header) {
	recv(client_socket, &(*header), sizeof(*header), 0);
}

void send_put(int client_socket, MsgPUT *msg_put) {
	
}

void recv_put(int client_socket, MsgHeader header, MsgPUT *msg) {
	/* 구조체 초기화 */
	//memset(&msg_put, 0, sizeof(msg_put));
	printf("fns, os, ds > %zu, %zu, %zu\n", header.file_name_size, header.owner_size, header.data_size);

	printf("init msg put\n");
	msg = malloc(sizeof(MsgPUT));
	printf("msg put done\n");
	msg->file_name 	= (char *)malloc(sizeof(char) * header.file_name_size);
	printf("file name done\n");
	msg->owner	 	= (char *)malloc(sizeof(char) * header.owner_size);
	printf("owner done\n");
	msg->data		= (char *)malloc(sizeof(char) * header.data_size);
	printf("data done\n");
	memcpy(&(msg->header), &header, sizeof(MsgHeader));

	/* 순서대로 recv */
	recv(client_socket, msg->file_name, msg->header.file_name_size, 0);
	recv(client_socket, msg->owner ,	msg->header.owner_size, 0);
	recv(client_socket, msg->data,		msg->header.data_size, 0);

	//printf("RECEVIED : %s, %s, %s\n", msg_put->file_name, msg_put->owner, msg_put->data);
}

void free_put(MsgPUT *msg_put) {
	free(msg_put->file_name);
	free(msg_put->owner);
	free(msg_put->data);
	free(msg_put);
}

void send_put_reply(int client_socket, MsgPUTREPLY *msg_put_reply) {
	
}

void recv_put_reply(int client_socket, MsgPUTREPLY *msg_put_reply) {

}

void free_put_reply(MsgPUTREPLY *msg_put_reply) {

}



/*
void set_header(MsgHeader *header, size_t body_size, size_t buff_size, MsgType msg_type) {
	memset(&(*header), 0, sizeof(MsgHeader));
	(*header).body_size = body_size;
	(*header).buff_size = buff_size;
	(*header).msg_type = msg_type;
}
*/


/*size_t get_file_size(char *file_name) {
	struct stat st; // (https://pubs.opoengroup.org/onlinepubs/7908799/xsh/stat.html)
	stat(file_name, &st); // write file info into buff(struct stat)
	size = st.st_size;
	return (size_t) size;	
}*/

