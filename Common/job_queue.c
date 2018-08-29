#include <stdio.h>

job_queue* init_job_queue(void);
void remove_job_queue(job_queue* jq);
void job_enqueue(job_queue* jq, job j);
int job_dequeue(job_queue* jq);
int is_empty(job_queue* jq);

job_queue*
init_job_queue(void)
{
	job_queue *jq 	= (job_queue *)malloc(sizeof(job_queue));
	jq->length 		= 0;
	jq->front		= NULL;
	jq->rear		= NULL;
	return jq;
}

void
remove_job_queue(job_queue* jq)
{
	while(jq->length > 0) {
		job_dequeue(jq);
	}
	free(jq);
}

void 
job_enqueue(job_queue *jq, job j)
{
	job_node *jn 		= (job_node *)malloc(sizeof(job_node));
	jn->j				= j;
	jn->next			= NULL;

	if(jq->length == 0) {
		jq->front 	= jn;
		jq->rear 	= jn;
	} else {
		jq->rear->next 		= jn;
		jq->rear 			= jn;
	}
	jq->length		= jq->length + 1;
}

int 
job_dequeue(job_queue *jq)
{
	int result;
	job_node* tmp_node;

	if(is_empty(jq)){
		printf("There is no job in job queue.\n");
		return;
	}

	tmp_node 	= jq->front;
	jq->front 	= tmp_node->next;
	if(tmp_node->next == NULL) {
		jq->rear = NULL;
	} else {
		jq->rear = tmp_node->next;
	}
	jq->length 	= jq->length - 1;
	
	result = tmp_node->j;
	free(tmp_node);
	
	return result;
}

int
is_empty(job_queue *jq)
{
	if(jq->length == 0)
		return 0;
	return -1;
}
