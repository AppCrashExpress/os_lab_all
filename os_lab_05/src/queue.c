#include <stdlib.h>
#include <stdio.h>
#include "MD5Queue.h"

int main()
{
    SumQueue sumQ;
    MD5Sum buff = {0, 0};
    sumsInit(&sumQ);

    printf("1: ");
    scanf("%llx", &buff.a);
    scanf("%llx", &buff.b);
    sumsPush(&sumQ, buff);

    printf("2: ");
    scanf("%llx", &buff.a);
    scanf("%llx", &buff.b);
    sumsPush(&sumQ, buff);

    printf("3: ");
    scanf("%llx", &buff.a);
    scanf("%llx", &buff.b);
    sumsPush(&sumQ, buff);

    printf("\n");
    while(!sumsEmpty(&sumQ))
    {
        buff = sumsPop(&sumQ);
        printf("%llx%llx\n", buff.a, buff.b);
    }

}