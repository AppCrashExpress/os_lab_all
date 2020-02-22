#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include "MD5Queue.h"

int main()
{
    void* libHandle = NULL;

    void   (*sumsInit) (SumQueue*) = NULL;
    void   (*sumsPush) (SumQueue*, MD5Sum) = NULL;
    int    (*sumsEmpty)(SumQueue*) = NULL;
    MD5Sum (*sumsPop)  (SumQueue*) = NULL;

    libHandle = dlopen("lib/libMD5Queue.so", RTLD_LAZY);
    if (!libHandle)
    {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    sumsInit  = dlsym(libHandle, "sumsInit");
    if (!sumsInit) {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    sumsPush  = dlsym(libHandle, "sumsPush");
    if (!sumsPush) {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    sumsEmpty = dlsym(libHandle, "sumsEmpty");
    if (!sumsEmpty) {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    sumsPop   = dlsym(libHandle, "sumsPop");
    if (!sumsPop) {
        printf("%s\n", dlerror());
        exit(EXIT_FAILURE);
    }
    
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

    dlclose(libHandle);
}