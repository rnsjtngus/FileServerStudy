#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct _test {
	char *c;
	uint32_t i;
}test;

int main(int argc, char *argv[]) {

	printf("%zu\n", sizeof(char));

	return 0;
}

