// File Description comment

// #include files
#include "common.h"
// Constant, static type definitions


// Global variables and functions exported
//void set_header(MsgHeader *header, size_t body_size, size_t buff_size, MessageType msg_type);

// variables and functions
void send_header(int client_socket, MsgHeader header) {
	send(client_socket, &header, sizeof(MsgHeader), 0);
}

void recv_header(int client_socket, MsgHeader *header) {
	recv(client_socket, &(*header), sizeof(MsgHeader), 0);
}

void send_put(int client_socket, MsgPUT *msg_put) {
	
}

void recv_put(int client_socket, MsgHeader header, MsgPUT *msg) {
	/* 구조체 초기화 */
	msg 			= malloc(sizeof(MsgPUT));
	msg->file_name 	= (char *)malloc(sizeof(char) * header.file_name_size);
	msg->owner	 	= (char *)malloc(sizeof(char) * header.owner_size);
	msg->data		= (char *)malloc(sizeof(char) * header.data_size);
	memcpy(&(msg->header), &header, sizeof(MsgHeader));

	/* 순서대로 recv */
	recv(client_socket, msg->file_name, (msg->header).file_name_size, 0);
	recv(client_socket, msg->owner ,	msg->header.owner_size, 0);
	recv(client_socket, msg->data,		msg->header.data_size, 0);
}

void free_put(MsgPUT *msg_put) {
	free(msg_put->file_name);
	free(msg_put->owner);
	free(msg_put->data);
	free(msg_put);
}

void send_put_reply(int client_socket, MsgPUTREPLY *msg_put_reply) {
	//send(client_socket, &(msg_put_reply->header), sizeof(MsgHeader), 0);	
	send_header(client_socket, msg_put_reply->header);
}

void recv_put_reply(int client_socket, MsgPUTREPLY *msg_put_reply) {

}

void free_put_reply(MsgPUTREPLY *msg_put_reply) {
	free(msg_put_reply);
}

void send_get(int client_socket, MsgGET *msg_get) { }
void recv_get(int client_socket, MsgHeader header, MsgGET *msg_get) { }
void free_get(MsgGET *msg_get) { }
void 
send_get_reply(int client_socket, int fd, MsgHeader header, char* file_name, MsgGETREPLY *msg_get_reply) 
{ 
	msg_get_reply = malloc(sizeof(MsgGETREPLY));
	msg_get_reply->file_name = (char *)malloc(sizeof(char) * header.file_name_size);
	msg_get_reply->owner = (char *)malloc(sizeof(char) * header.owner_size);
	msg_get_reply->data = (char *)malloc(sizeof(char) * header.data_size);

	memcpy(&(msg_get_reply->header), &header, sizeof(header));
	memcpy(msg_get_reply->file_name, file_name, sizeof(char) * header.file_name_size);
	read(fd, msg_get_reply->data, sizeof(char) * header.data_size);

	send(client_socket, &(msg_get_reply->header), sizeof(MsgHeader), 0);
	send(client_socket, msg_get_reply->file_name, header.file_name_size, 0);
	send(client_socket, msg_get_reply->owner, header.owner_size, 0);
	send(client_socket, msg_get_reply->data, header.data_size, 0);
}

void 
recv_get_reply(int client_socket, MsgHeader header, MsgGETREPLY *msg_get_reply) 
{
	msg_get_reply 				= malloc(sizeof(MsgGETREPLY));
	msg_get_reply->file_name 	= (char *)malloc(sizeof(char) * header.file_name_size);
	msg_get_reply->owner 		= (char *)malloc(sizeof(char) * header.owner_size);
	msg_get_reply->data 		= (char *)malloc(sizeof(char) * header.data_size);
	memcpy(&(msg_get_reply->header), &header, sizeof(MsgHeader));

	recv(client_socket, msg_get_reply->file_name, msg_get_reply->header.file_name_size, 0);
	recv(client_socket, msg_get_reply->owner, msg_get_reply->header.owner_size, 0);
	recv(client_socket, msg_get_reply->data, msg_get_reply->header.data_size, 0);
}

void 
free_get_reply(MsgGETREPLY *msg_get_reply) 
{
	free(msg_get_reply->data);
	free(msg_get_reply->owner);
	free(msg_get_reply->file_name);
	free(msg_get_reply);
}

void send_list(int client_socket, MsgLIST *msg_list) { }
void recv_list(int client_socket, MsgHeader header, MsgGET *msg_get) { }
void free_list(MsgLIST msg_list) { }
void send_list_reply(int client_socket, MsgLISTREPLY *msg_list_reply) { }

void 
recv_list_reply(int client_socket, MsgHeader header, MsgLISTREPLY *msg_list_reply) 
{ 
	msg_list_reply 				= malloc(sizeof(MsgLISTREPLY));
	msg_list_reply->file_name 	= (char *)malloc(sizeof(char) * header.file_name_size);
	memcpy(&(msg_list_reply->header), &header, sizeof(MsgHeader));

	recv(client_socket, msg_list_reply->file_name, msg_list_reply->header.file_name_size, 0);
}

void 
free_list_reply(MsgLISTREPLY *msg_list_reply) 
{ 
	free(msg_list_reply->file_name);
	free(msg_list_reply);
}

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

