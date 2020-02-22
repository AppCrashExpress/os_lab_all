#ifndef PAIR_QUEUE_H
#define PAIR_QUEUE_H
#define MAX 100

typedef struct adjacent
{
    int i, j;
} adjacent;

typedef struct PairQueue {
    int size;
    int first;
    adjacent array[MAX];
} PairQueue;


void pairsInit(PairQueue*);

int  pairsSize(PairQueue*);
int  pairsEmpty(PairQueue*);
int  pairsFull(PairQueue*);
adjacent* pairsPeek(PairQueue*);

adjacent pairsPop(PairQueue*);
void pairsPush(PairQueue*, adjacent);


#endif

