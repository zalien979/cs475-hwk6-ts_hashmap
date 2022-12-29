all: main.c ts_hashmap.o
	gcc -Wall -o hashtest main.c ts_hashmap.o -lpthread

ts_hashmap.o: ts_hashmap.h ts_hashmap.c
	gcc -Wall -c ts_hashmap.c

clean:
	rm -f hashtest *.o
