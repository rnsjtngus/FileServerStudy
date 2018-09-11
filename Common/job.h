typedef enum _job_operation_t {
	J_OP_AIO_WRITE,
	J_OP_AIO_WRITE_POST,
	J_OP_AIO_READ,
	J_OP_AIO_READ_POST,
	J_OP_LIST,
	J_OP_LIST_POST,
} j_op_t;

typedef struct _job_t {
	j_op_t 	operation;
	int		client_socket;
	int		fd;
	char*	buff;
	int		buff_size;
	int		offset;
	int*	count;
} job_t;

job_t* init_job(j_op_t op, int cs, int fd, int offset, char* buff, int buff_size, int* count);
void remove_job(job_t *job);
