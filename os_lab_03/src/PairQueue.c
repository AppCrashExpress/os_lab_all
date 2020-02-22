#include "PairQueue.h"

void pairsInit(PairQueue *Q)
{
    Q->size = Q->first = 0;
}

int pairsSize(PairQueue *Q)
{
    return Q->size;
}

int pairsEmpty(PairQueue *Q)
{
    return Q->size == 0 ? 1 : 0;
}

int pairsFull(PairQueue *Q)
{
    return Q->size == MAX ? 1 : 0;
}

adjacent* pairsPeek(PairQueue *Q)
{
    return &Q->array[Q->first];
}

adjacent pairsPop(PairQueue *Q)
{
    int selected = Q->first;
    --Q->size;
    Q->first = (Q->first + 1) % MAX;
    return Q->array[selected];
}

void pairsPush(PairQueue *Q, adjacent add)
{
    Q->array[(Q->first + Q->size) % MAX] = add;
    ++Q->size;
}
