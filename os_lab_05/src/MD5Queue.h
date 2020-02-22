#ifndef MD5_QUEUE_H
#define MD5_QUEUE_H
#define MAX 100

typedef struct {
    unsigned long long a;
    unsigned long long b;
} MD5Sum;
typedef struct SumQueue {
    int size;
    int first;
    MD5Sum array[MAX];
} SumQueue;


void  sumsInit (SumQueue*);

int   sumsSize (SumQueue*);
int   sumsEmpty(SumQueue*);
int   sumsFull (SumQueue*);

MD5Sum sumsPeek (SumQueue*);
MD5Sum sumsPop  (SumQueue*);
void   sumsPush (SumQueue*, MD5Sum);


#endif

