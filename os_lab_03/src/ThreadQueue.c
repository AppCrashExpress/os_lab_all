#include "ThreadQueue.h"

void threadsInit(ThreadQueue *Q)
{
    Q->size = Q->first = 0;
}

int threadsSize(ThreadQueue *Q)
{
    return Q->size;
}

int threadsEmpty(ThreadQueue *Q)
{
    return Q->size == 0 ? 1 : 0;
}

int threadsFull(ThreadQueue *Q)
{
    return Q->size == MAX ? 1 : 0;
}

pthread_t* threadsPeek(ThreadQueue *Q)
{
    return &Q->array[Q->first];
}

pthread_t threadsPop(ThreadQueue *Q)
{
    int selected = Q->first;
    --Q->size;
    Q->first = (Q->first + 1) % MAX;
    return Q->array[selected];
}

pthread_t* threadsPush(ThreadQueue *Q)
{
    // ++Q->size;
    return &Q->array[(Q->first + Q->size++) % MAX];
}
