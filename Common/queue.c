#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

queue_t*
init_queue(void)
{
	queue_t *q 		= (queue_t *)malloc(sizeof(queue_t));
	q->length 		= 0;
	q->front		= NULL;
	q->rear			= NULL;
	return q;
}

void
remove_queue(queue_t *q)
{
	while(q->length > 0) {
		dequeue(q);
	}
	free(q);
}

void
enqueue(queue_t *q, void* v)
{
	node_t *node	= (node_t *)malloc(sizeof(node_t));
	node->p 		= v;
	node->next		= NULL;

	if(q->length == 0) {
		q->front 	= node;
	} else {
		q->rear->next = node;
	}
	q->rear 		= node;
	q->length 		= q->length + 1;
}

void* 
dequeue(queue_t *q)
{
	void* result;
	node_t* tmp_node;

	if(is_empty(q)) {
		printf("There is no node in queue\n");
		return NULL;
	}
	
	tmp_node	= q->front;
	q->front 	= tmp_node->next;
	
	if(tmp_node->next == NULL) {
		q->rear	= NULL;
	}
	
	q->length 	= q->length - 1;

	result 		= tmp_node->p;
	free(tmp_node);

	return result;
}

int
is_empty(queue_t *q)
{
	if(q->length == 0)
		return 0;
	return 1;
}
