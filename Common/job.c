#include <stdio.h>
#include <stdlib.h>
#include "job.h"
	
job_t*
init_job(j_op_t op, int cs, int fd, char* buff, int buff_size, int offset, int* count)
{
	job_t* job 			= (job_t *)malloc(sizeof(job_t));
	job->operation		= op;
	job->client_socket	= cs;
	job->fd				= fd;
	job->buff 			= buff;
	job->buff_size		= buff_size;
	job->offset			= offset;
	job->count			= count;

	return job;
}

void
remove_job(job_t *job)
{
	free(job->buff);
	if(&(job->count) == 0) {
		free(job->count);
	}
	free(job);
}
