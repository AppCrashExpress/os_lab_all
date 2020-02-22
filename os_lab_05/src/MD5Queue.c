#include "MD5Queue.h"

void sumsInit(SumQueue *Q)
{
    Q->size = Q->first = 0;
}

int sumsSize(SumQueue *Q)
{
    return Q->size;
}

int sumsEmpty(SumQueue *Q)
{
    return Q->size == 0 ? 1 : 0;
}

int sumsFull(SumQueue *Q)
{
    return Q->size == MAX ? 1 : 0;
}

MD5Sum sumsPeek(SumQueue *Q)
{
    return Q->array[Q->first];
}

MD5Sum sumsPop(SumQueue *Q)
{
    int selected = Q->first;
    --Q->size;
    Q->first = (Q->first + 1) % MAX;
    return Q->array[selected];
}

void sumsPush(SumQueue *Q, MD5Sum add)
{
    Q->array[(Q->first + Q->size) % MAX] = add;
    ++Q->size;
}
