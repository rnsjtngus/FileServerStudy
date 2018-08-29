typedef struct _job {
	int client_socket;
} job;

typedef struct _job_node {
	job* job;
	job_node* next;
} job_node;

typedef struct _job_queue {
	int length;
	job_node *front;
	job_node *rear;
} job_queue;

job_queue* init_job_queue(void);
void remove_job_queue(job_queue* jq);
void job_enqueue(job_queue* jq, job* j);
int job_dequeue(job_queue* jq);
int is_empty(job_queue* jq);
