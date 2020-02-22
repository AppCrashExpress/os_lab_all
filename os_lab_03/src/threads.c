#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include "PairQueue.h"
#include "ThreadQueue.h"

#include <stdio.h>
typedef enum bool {false, true} bool;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
PairQueue pairs;

typedef struct vertex {
    bool adj;
    bool visited;
} vertex;

vertex **AdjMatr = NULL;
int MatrSize = 0;
void FillAdjacency (int fd)
{
    int ByteRead = 0;
    char x = '\0';

    if ( (ByteRead = read(fd, &x, 1)) <= 0)
    {
        write(STDOUT_FILENO, "Failed to read size\n", 21);
        exit(1);
    }

    do
    {
        MatrSize = MatrSize * 10 + (x - '0');
    } while ( (ByteRead = read(fd, &x, 1) > 0) && x != '\n');

    int i = 0, j = 0;

    AdjMatr = malloc(sizeof(vertex *) * MatrSize);
    for (i = 0; i < MatrSize; ++i)
        AdjMatr[i] = malloc(sizeof(vertex) * MatrSize);

    for (i = 0; i < MatrSize; ++i)
        for (j = 0; j < MatrSize; ++j)
        {
            AdjMatr[i][j].adj = false;
            AdjMatr[i][j].visited = false;
        }


    
    while ( (ByteRead = read(fd, &x, 1)) > 0)
    {
        i = j = 0;
        if (x == '\n' || x == ' ') continue;

        do {
            i = i * 10 + (x - '0');
        } while ( (ByteRead = read(fd, &x, 1) > 0) && x != ' ');

        read(fd, &x, 1); // Skip whitespace
        
        do {
            j = j * 10 + (x - '0');
        } while ( (ByteRead = read(fd, &x, 1) > 0) && x != '\n');

        AdjMatr[i][j].adj = AdjMatr[j][i].adj = true;
    }
}

void * BreadthFirstSearch(void *args)
{
    adjacent *pair = (adjacent *) args;
    int iT = pair->j; //as in i Transpose
    int jT = pair->i; //as in j Transpose
    
    for (int i = 0; i < MatrSize; ++i)
    {
        if (i == iT) continue;
        if (AdjMatr[i][jT].adj)
        {
            pthread_mutex_lock(&mutex);
            if (AdjMatr[i][jT].visited)
            {
                pthread_mutex_unlock(&mutex);
                pthread_exit((void *) true);
            }            

            AdjMatr[i][jT].visited = true;
            adjacent pair = {i, jT};
            pairsPush(&pairs, pair);

            pthread_mutex_unlock(&mutex);
        }
    }
    pthread_exit((void *) false);
}

int main(int argc, char *argv[])
{
    int fd;
    if (argc > 1)
        fd = open(argv[1], O_RDONLY);
    else
    {
        write(STDOUT_FILENO, "Specify file as a first parameter\n", 35);
        exit(1);
    }
    if (fd < 0)
    {
        write(STDOUT_FILENO, "File failed to open\n", 21);
        exit(-1);
    }
    FillAdjacency(fd);
    close(fd);

    int i;
    for (i = 0; i < MatrSize; ++i)
    {
        for (int j = 0; j < MatrSize; ++j)
            printf("%d ", AdjMatr[i][j].adj);
        printf("\n");
    }

    pairsInit(&pairs);
    adjacent pair;
    int MaxThreads = 0;
    if (argc > 2)
    {
        i = 0;
        while (argv[2][i] != '\0')
        {
            MaxThreads = MaxThreads * 10 + (argv[2][i] - '0');
            ++i;
        }
    }
    else
        MaxThreads = MatrSize;
    if (MaxThreads <= 0 || MaxThreads > MatrSize)
    {
        write(STDOUT_FILENO, "Invalid thread amount\n", 23);
        exit(-1);
    }


    for (i = 0; i < MatrSize; ++i)
    {
        if (AdjMatr[i][0].adj)
        {
            pair.i = i;
            pair.j = 0;
            pairsPush(&pairs, pair);    
        }
    }
    AdjMatr[0][0].visited = true;

    ThreadQueue threads;
    threadsInit(&threads);
    while (!pairsEmpty(&pairs) && threadsSize(&threads) < MaxThreads)
    {
        pthread_create(threadsPush(&threads), NULL, BreadthFirstSearch, (void *) pairsPeek(&pairs));
        pairsPop(&pairs);
    }

    void *status;
    bool looped = false;
    while (!threadsEmpty(&threads))
    {
        pthread_join(threadsPop(&threads), &status);
        if ((bool *) status) looped = true;
        while (!pairsEmpty(&pairs) && threadsSize(&threads) < MaxThreads)
        {
            pthread_create(threadsPush(&threads), NULL, BreadthFirstSearch, (void *) pairsPeek(&pairs));
            pairsPop(&pairs);
        }
    }

    for (i = 0; i < MatrSize; ++i)
        free(AdjMatr[i]);
    free(AdjMatr);
    pthread_mutex_destroy(&mutex);

    if (looped)
        write(STDOUT_FILENO, "The graph contains loop\n", 25);
    else if (pairsEmpty(&pairs))
        write(STDOUT_FILENO, "No loops found\n", 16);
    else 
    {
        write(STDOUT_FILENO, "what\n", 6);
        return 1;
    }
    return 0;
}
