
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
		return 1;
	}

  srand(time(NULL));
	int num_threads = atoi(argv[1]);
	int capacity = (unsigned int) atoi(argv[2]);

	// TODO: Write your test

	return 0;
}

