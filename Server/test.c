#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct _test {
	char *c;
	uint32_t i;
}test;

int main(int argc, char *argv[]) {
	char ss[100];
	printf("%zu\n", sizeof(ss));
	char *s = (char *)malloc(sizeof(char) * 102);
	printf("%zu\n", sizeof(&s));
	
	test *t; // = malloc(sizeof(test));
	t = (test *) malloc (sizeof(test));
	t->c = (char *)malloc(sizeof(char) * 107);

	printf("%zu, %zu, %zu, %zu\n", sizeof(test), sizeof(t), sizeof(t->c), sizeof(t->i));

	free(t->c);
	free(t);

	return 0;
}

