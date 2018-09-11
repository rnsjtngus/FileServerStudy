#include "message.h"

void 
msg_send(int socket_fd, const void *msg, size_t len, int flags)
{
	send(socket_fd, msg, len, flags);
	return;
}

void 
msg_recv(int socket_fd, void *buff, size_t len, int flags)
{
	recv(socket_fd, buff, len, flags);
	return;
}

void
msg_meta_recv(int socket_fd, msg_meta_t *msg_meta)
{
	msg_meta = (msg_meta_t *)malloc(sizeof(msg_meta_t));

	msg_recv(socket_fd, msg_meta, sizeof(msg_meta) - sizeof(char *) * 2, 0);

	msg_meta->file_name = (char *)malloc(sizeof(char) * msg_meta->file_name_size);
	msg_meta->owner_name = (char *)malloc(sizeof(char) * msg_meta->owner_name_size);

	msg_recv(socket_fd, msg_meta->file_name, msg_meta->file_name_size, 0);
	msg_recv(socket_fd, msg_meta->owner_name, msg_meta->owner_name_size, 0);

	return;
}

void
msg_meta_free(msg_meta_t *msg_meta)
{
	free(msg_meta->file_name);
	free(msg_meta->owner_name);
	free(msg_meta);
	return;
}
