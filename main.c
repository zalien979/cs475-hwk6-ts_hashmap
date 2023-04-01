
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
	if (num_threads<1){
		printf("please enter a number higher than 0 for threads");
		return 1;
	}
	if (capacity<1){
		printf("please enter a number higher than 0 for capacity");
		return 1;
	}
	ts_hashmap_t *hashmap=initmap(capacity);
	put(hashmap, 3,5);
	put(hashmap, 33,3);
	put(hashmap, 34,57);
	put(hashmap, 8,6);
	put(hashmap, 7, 8);
	put(hashmap, 12, 45);
	get(hashmap, 3);
	del(hashmap, 3);
	printmap(hashmap);
	pthread_mutex_destroy(hashmap->sizelock);
	for(int i=0; i<capacity; i++){
		pthread_mutex_destroy(hashmap->locks[i]);
	}
	return 0;
}