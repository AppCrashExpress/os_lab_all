#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include <string.h>
#include <fcntl.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

const size_t BUFF_SIZE = 100;
const size_t CharSize  = sizeof(char);
const char FILE_NAME[] = "c943a334af5f2cc790749b07e3a85733";

int main()
{
    int fd;
    if ( (fd = open(FILE_NAME, O_CREAT | O_RDWR)) < 0)
    {
        printf("Failed to create temporary file(\"%s\")\n", FILE_NAME);
        exit(EXIT_FAILURE);
    }
    if (posix_fallocate(fd, 0, PAGE_SIZE) < 0)
    {
        printf("File allocation failure(\"%s\")\n", FILE_NAME);
        exit(EXIT_FAILURE);
    }
    char *mappedFile;
    if ((mappedFile = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        perror("Failed to map file into virtual memory\n");
        remove(FILE_NAME);
        exit(EXIT_FAILURE);
    }
        //Improvise. Adapt. Overcome.
    int *crappyMutex;
    if ((crappyMutex = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fd, 0)) == MAP_FAILED)
    {
        perror("Failed to create mutex\n");
        remove(FILE_NAME);
        exit(EXIT_FAILURE);
    }

    *crappyMutex = 1;

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("Failed to execute\n");
        remove(FILE_NAME);
        exit(EXIT_FAILURE);
    }
    if (pid == 0)
    {
        char com[BUFF_SIZE];
        int i = 0;
            //Get the command from parrent
        while (*crappyMutex) { ;}
        while (i != BUFF_SIZE)
        {
            com[i] = mappedFile[i];
            mappedFile[i] = '\0';
            if (com[i] == '\0')
            {
                ++i;
                break;
            }
            ++i;
        }

        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
            //Use the command and sent result to parent
        char *result[] = {"sh", "-c", com, NULL};
        execv("/bin/sh", result);

    }
    else
    {
        char ComIn[BUFF_SIZE];
        int i = 0;
        do {
            scanf("%c", &ComIn[i]);
            if (ComIn[i] == '\n')
            {
                ++i;
                break;
            }
            ++i;
        } while (i != BUFF_SIZE);

            //Write the command to child
        for (int j = 0; j < i; ++j)
            { mappedFile[j] = ComIn[j]; }
        mappedFile[i + 1] = '\0';
        *crappyMutex = 0;
            //Wait for child
        waitpid(pid, NULL, 0);
            //Parsing process
        for (i = 0; mappedFile[i] != '\0'; ++i)
        {
            if (mappedFile[i] == ' ') printf("_");
            else if (mappedFile[i] == '\t') printf("___");
            else printf("%c", mappedFile[i]);
        }
    }
    remove(FILE_NAME);
    munmap(mappedFile,  PAGE_SIZE);
    munmap(crappyMutex, sizeof(int));
    return EXIT_SUCCESS;
}
