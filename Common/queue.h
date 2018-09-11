typedef struct _node_t {
	void* 	p;
	struct _node_t 	*next;
} node_t;

typedef struct _queue_t {
	int 	length;
	node_t 	*front;
	node_t 	*rear;
} queue_t;

queue_t* init_queue(void);
void remove_queue(queue_t* q);
void enqueue(queue_t* q, void* v);
void* dequeue(queue_t* q);
int is_empty(queue_t* q);
