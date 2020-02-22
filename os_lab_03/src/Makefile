.PHONY: all
all: GraphScan

GraphScan: Threads
	gcc threads.o PairQueue.o ThreadQueue.o -lpthread -g -o GraphScan

Threads: threads.c PairQueue ThreadQueue
	gcc -lpthread threads.c -c

PairQueue: PairQueue.c PairQueue.h
	gcc PairQueue.c -c 

ThreadQueue: ThreadQueue.c ThreadQueue.h
	gcc -lpthread ThreadQueue.c -c 

.PHONY: clean
clean:
	rm *.o GraphScan
