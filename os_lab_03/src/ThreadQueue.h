#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H
#define MAX 100

#include <pthread.h>

typedef struct ThreadQueue {
    int size;
    int first;
    pthread_t array[MAX];
} ThreadQueue;


void threadsInit(ThreadQueue*);

int  threadsSize(ThreadQueue*);
int  threadsEmpty(ThreadQueue*);
int  threadsFull(ThreadQueue*);
pthread_t* threadsPeek(ThreadQueue*);

pthread_t  threadsPop(ThreadQueue*);
pthread_t* threadsPush(ThreadQueue*);


#endif

