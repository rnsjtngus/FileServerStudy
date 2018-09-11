#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <libaio.h>

#define MAX_FILE_NAME_SIZE 1024

int main() {
	char* user_name;
	char* result;

	user_name = (char *)malloc(sizeof(char) * MAX_FILE_NAME_SIZE);

	getlogin_r(user_name, MAX_FILE_NAME_SIZE);
	//cuserid(user_name);

	printf("user_name > %s\n", user_name);
	printf("user_name length > %d\n", strlen(user_name));
	
	return 0;
}

