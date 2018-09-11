#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <string.h>
#include <fcntl.h> /* for the open options */
#include <unistd.h>
#include <dirent.h>
#include <libaio.h>
//#include <linux/aio_abi.h>
#include <pthread.h>
//#include <linux/aio_abi.h>
//#include <linux/time.h>

#include "../Common/common.h"
#include "../Common/job.h"
#include "../Common/queue.h"

#define MAX_BUFF_SIZE 1024

#define EVENT_MAX_VALUE 1024
#define BUFF_SIZE 1024
#define PORT 7777
#define SAVE_DIR "./Files/"

void set_server_socket(int *server_socket, struct sockaddr_in *server_addr);
void server_put(int client_socket, msg_meta_t *msg_meta, queue_t *job_queue);
void reply_put(int client_socket);
char* get(int clinet_socket, MsgHeader h);
void reply_get(int client_socket, char* file_name);
void list(int client_socket, MsgHeader header);
void reply_list(int client_socket);

void* wthr_work(void* jq);
void submit_write_aio(job_t* job);
void submit_read_aio(job_t* job);
void* sthr_work(void* jq);


io_context_t ctx;

int 
main(char *args, char *argv[]) 
{
	/* 변수 선언 */
	int server_socket;
	int client_socket;
	int client_addr_size;
	int pid, fd;
	pthread_t wthr, sthr;
	int wthr_id, sthr_id;

	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	char *file_name;

	msg_meta_t *msg_meta;

	queue_t* job_queue;

	/* io context 초기화 */
	memset(&ctx, 0, sizeof(ctx));
	if(io_setup(10, &ctx) != 0) err(1, "io_setup");

	/* job queue 초기화 */
	job_queue = init_queue();

	/* wthr, sthr 초기화 */
	wthr_id = pthread_create(&wthr, NULL, wthr_work, job_queue);
	sthr_id = pthread_create(&sthr, NULL, sthr_work, job_queue);

	printf("wthr id, sthr id : %d, %d\n", wthr, sthr);

	/* 서버 소켓 생성 부터 listen 상태까지 세팅 */
	set_server_socket(&server_socket, &server_addr);

	client_addr_size = sizeof(client_addr);
	client_socket = accept(server_socket, 
						(struct sockaddr*) &client_addr, 
						&client_addr_size);

	if(client_socket == -1) {
		printf("Fail to connect with client\n");
		exit(1);
	}
	
	/*
	memset(&header, 0, sizeof(MsgHeader));
	recv_header(client_socket, &header);
	*/

	/* recv msg meta */
	msg_meta_recv(client_socket, msg_meta);

	while(1){
		client_addr_size = sizeof(client_addr);
		client_socket = accept(server_socket, 
							(struct sockaddr*) &client_addr, 
							&client_addr_size);

		if(client_socket == -1) {
			printf("Fail to connect with client\n");
			exit(1);
		}
	
		/*
		memset(&header, 0, sizeof(MsgHeader));
		recv_header(client_socket, &header);
		*/
	
		/* recv msg meta */
		msg_meta_recv(client_socket, msg_meta);

		switch(msg_meta->type) {
			case MSG_PUT:
				put(client_socket, msg_meta, job_queue); 
				break;
			case MSG_GET:
				get(client_socket, header);
				break;
			case MSG_LIST:
				break;
			default:
				printf("Received wrong message.\n");
				break;
		}	
		close(client_socket);	
	}
	io_destroy(ctx);

	/* free malloc */
	remove_queue(job_queue);

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
server_put(int client_socket, msg_meta_t *msg_meta, queue_t *job_queue)
{
	int fd, i;
	int *count;
	char* buff;
	msg_put_t *msg_put;
	job_t *j;

	/* open file */
	if((fd = open(msg_meta->file_name, O_WRONLY | O_CREAT, R_IRWXU)) == -1) {
		printf("[PUT] Fail to open file.\n");
		return;
	}
	
	/* recv msg put & make job & enqueue job */
	count = (int *)malloc(sizeof(int));
	*count = msg_meta->total_offset;
	for(i = 0; i < msg_meta->total_offset; i++) {
		/* init & recv msg put */
		msg_put = (msg_put_t *)malloc(sizeof(msg_put_t));
		msg_recv(client_socket, msg_put, sizeof(msg_put_t) - sizeof(char *), 0);

		msg_put->buff = (char *)malloc(sizeof(char) * msg_put->buff_size);
		msg_recv(client_socket, msg_put->buff, sizeof(char) * msg_put->buff_size, 0);

		/* copy buff */
		buff = (char *)malloc(sizeof(char) * msg_put->buff_size);
		memcpy(buff, msg_put->buff, sizeof(char) * msg_put->buff_size);

		/* make job and enqueue to job queue */
		j = init_job(J_OP_AIO_WRITE, client_socket, fd, 
				msg_put->offset * msg_meta->max_buff_size,
				msg_put->buff_size, buff, count);
		
		enqueue(job_queue, j);

		/* free msg put */
		free(msg_put->buff);
		free(msg_put);
	}
	
	/* free msg meta */
	msg_meta_free(msg_meta);

	return;
}

void
server_get(int client_socket, msg_meta_t *msg_meta, queue_t *job_queue)
{
	int fd, file_size, total_offset, buff_size, i;
	int* count;	
	char* buff;
	job_t *job;

	/* open file and check file exist */
	if((fd = open(msg_meta->file_name, O_RDONLY, S_IRWXU)) == -1) {
		printf("[GET] Fail to open file.\n");
		return;
	}

	file_size = get_file_size(msg_meta->file_name);
	total_offset = cceil(((double) file_size) / MAX_BUFF_SIZE);
	
	count = (int *)malloc(sizeof(int));
	*count = total_offset;

	/** TODO : send get reply fisrt?????? **/

	for(i = 0; i < total_offset; i++) {
		/* init job */
		buff_size = (rest > MAX_BUFF_SIZE) ? MAX_BUFF_SIZE : rest;
		job = init_job(J_OP_AIO_READ, client_socket, fd,
				i * MAX_BUFF_SIZE, buff, buff_size, count);

		enqueue(job_queue, j);
	}
	/* free msg put */
	msg_meta_free(msg_meta);

	return;
}



/*
void 
put(int client_socket, MsgHeader h) 
{
	int fd;
	MsgPUT *msg_put;
	uint32_t rest;
	MsgHeader header;

	msg_put = malloc(sizeof(MsgPUT));
	memcpy(&header, &h, sizeof(header));

	printf("START TO RECV\n");
	recv_put(client_socket, header, msg_put);

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
	free(msg_put);
}

void 
reply_put(int client_socket) 
{
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

	msg_get = malloc(sizeof(MsgGET));
	recv_get(client_socket, header, msg_get);

	file_name = (char *)malloc(sizeof(char) * header.file_name_size);
	memcpy(file_name, msg_get->file_name, msg_get->header.file_name_size);
	
	free_get(msg_get);
	free(msg_get);

	printf("FILE NAME : %s\n", file_name);
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

	printf("FILE NAME : %s!\n", file_name);

	if((fd = open(file_name, O_RDONLY)) == -1) {
		printf("Cant open file > %s\n", file_name);
		return;
	}

	file_size = get_file_size(file_name);
	total_offset = cceil(((double) file_size) / BUFF_SIZE);
	
	msg_get_reply = malloc(sizeof(MsgGETREPLY));

	memset(&header, 0, sizeof(header));
	header.file_name_size = strlen(file_name);
	header.owner_size = 0;
	header.total_offset = total_offset;
	header.file_size = file_size;

	rest = file_size;
	printf("Start to send files\n");
	for(i = 0; i < total_offset; i++) {
		printf("%d\n", i);
		header.offset = i;
		header.data_size = (rest > BUFF_SIZE) ? BUFF_SIZE : rest;

		rest = (rest > BUFF_SIZE) ? rest - BUFF_SIZE : 0;

		send_get_reply(client_socket, fd, file_name, header, msg_get_reply);
		free_get_reply(msg_get_reply);
	}
	free(msg_get_reply);
	close(fd);
}

void
list(int client_socket, MsgHeader header)
{
	MsgLIST *msg_list;
	
	msg_list = malloc(sizeof(MsgLIST));
	
	printf("Start to recv list\n");

	recv_list(client_socket, header, msg_list);

	free_list(msg_list);
	free(msg_list);
	printf("Done\n");
	
}

void 
reply_list(int client_socket)
{
	MsgHeader header;
	MsgLISTREPLY *msg_list_reply;

	struct dirent *de;
	
	DIR *dr = opendir(".");

	int offset, total_offset = 0;

	printf("Start to reply list\n");

	if(dr == NULL) {
		printf("Could not open current directory.\n");
		return;
	}

	memset(&header, 0, sizeof(MsgHeader));
	
	printf("Check file numbers\n");

	while((de = readdir(dr)) != NULL) {
		total_offset = total_offset + 1;
	}
	closedir(dr);
	printf("Total files : %d\n", total_offset);
	printf("Start to send list\n");

	dr = opendir(".");
	header.total_offset = total_offset;	
	offset = 0;

	while((de = readdir(dr)) != NULL) {
		printf("Sending %dth file name : %s\n", offset, de->d_name);
		header.file_name_size = strlen(de->d_name);
		header.offset = offset;

		msg_list_reply = malloc(sizeof(MsgLISTREPLY));
		msg_list_reply->file_name = (char *)malloc(sizeof(char) * strlen(de->d_name));

		memcpy(&(msg_list_reply->header), &header, sizeof(MsgHeader));
		memcpy(msg_list_reply->file_name, de->d_name, sizeof(char) * strlen(de->d_name));

		send(client_socket, &(msg_list_reply->header), sizeof(MsgHeader), 0);
		send(client_socket, msg_list_reply->file_name, sizeof(char) * strlen(de->d_name), 0);

		free(msg_list_reply->file_name);
		free(msg_list_reply);

		offset = offset + 1;
	}

	closedir(dr);
	printf("Done\n");

	return;
}
*/

void*
wthr_work(void* jq)
{
	queue_t *job_queue = (queue_t *)jq;
	job_t *j;
	while(1) {
		if(!is_empty(job_queue)) {
			j = (job_t *)dequeue(job_queue);
			switch(j->operation) {
			case J_OP_AIO_WRITE:
				submit_write_aio(j);
				break;
			case J_OP_AIO_READ:
				submit_read_aio(j);
				break;
			case J_OP_LIST:
				break;
			case J_OP_AIO_WRITE_POST:
				break;
			case J_OP_AIO_READ_POST:
				break;
			case J_OP_LIST_POST:
				break;
			default:
				break;
			}
		}
	}
}

void
submit_write_aio(job_t* j)
{
	struct iocb iocb;
	struct iocb* iocbs[1];

	io_prep_pwrite(&iocb, j->fd, j->buff, j->buff_size, j->offset);
	iocb.data = (void *) j;

	iocbs[0] = &iocb;

	if(io_submit(ctx, 1, iocbs) != 1) {
		io_destroy(ctx);
		err(1, "[submit_write_aio] io_submit");
	}
}

void
submit_read_aio(job_t* j)
{
	struct iocb iocb;
	struct iocb* iocbs[1];

	io_prep_pread(&iocb, j->fd, j->buff, j->buff_size, j->offset);
	iocb.data = (void *) j;

	iocbs[0] = &iocb;

	if(io_submit(ctx, 1, iocbs) != 1) {
		io_destroy(ctx);
		err(1, "[submit_read_aio] io_submit");
	}
}

void*
sthr_work(void* jq)
{
	queue_t *job_queue = (queue_t *)jq;

	struct io_event events[EVENT_MAX_VALUE];
	struct timespec timeout;
	struct iocb* iocb;

	int events_num, i;
	job_t* j;

	timeout.tv_sec = 0;
	timeout.tv_nsec = 100000000;

	while(1) {
		if((events_num = io_getevents(ctx, 0, 10, events, &timeout)) > 1) {
			for(i = 0; i < events_num; i++) {
				iocb = (struct iocb *) events[i].obj;
				j = (job_t *) events[i].data;
				
				switch(j->operation) {
				case J_OP_AIO_WRITE:
					j->operation = J_OP_AIO_WRITE_POST;
					enqueue(job_queue, j);
					break;
				case J_OP_AIO_READ:
					j->operation = J_OP_AIO_READ_POST;
					enqueue(job_queue, j);
					break;
				default:
					printf("Something wrong\n");
					break;
				}
			}
		}
	}
}
