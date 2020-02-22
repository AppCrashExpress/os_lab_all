#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

const size_t BUFF_SIZE = 100;
const size_t CharSize = sizeof(char);

int main()
{
  int fd1[2]; //From parent to child 
  int fd2[2]; //From child to parent

  if (pipe(fd1) == -1)
  {
    write(STDOUT_FILENO, "Pipe creation failure.\n", sizeof("Pipe creation failure.\n")); //sizeof string always includes terminator. We don't need it.
    exit(EXIT_FAILURE);
  }
  if (pipe(fd2) == -1)
  {
    write(STDOUT_FILENO, "Pipe creation failure.\n", sizeof("Pipe creation failure.\n"));
    exit(EXIT_FAILURE);
  }

  pid_t pid = fork();
  if (pid == -1)
  {
    write(STDOUT_FILENO, "Failed to execute.\n", sizeof("Failed to execute.\n")); 
    exit(EXIT_FAILURE);
  }
  if (pid == 0)
  {
    int size;
    char com[BUFF_SIZE];
    //Get the command from parrent
    read(fd1[0], &size, sizeof(int));
    read(fd1[0], com, size);
    close(fd2[0]);

    dup2(fd2[1], STDOUT_FILENO);
    dup2(fd2[1], STDERR_FILENO);
    //Use the command and sent result to parent
    char *result[] = {"sh", "-c", com, NULL};
    execv("/bin/sh", result);
    close(fd2[1]);
  }
  else
  {
    close(fd1[0]); //We don't need to read  from parent->child pipe
    close(fd2[1]); //We don't need to write to   child->parent pipe

    int ByteRead = 0, i = 0;
    char ComIn[BUFF_SIZE];
    while( (ByteRead = read(STDIN_FILENO, &ComIn[i], CharSize)) > 0 ) // Get the command
    {
      if (ComIn[i] == '\n') close(STDIN_FILENO);
      ++i;
    }
    ComIn[i++] = '\0'; // and close it
      //Write the command to child
    write(fd1[1], &i, sizeof(int));
    write(fd1[1], ComIn, i);
    close(fd1[1]);
      //Wait for child
    waitpid(pid, NULL, 0);
      //Parsing process
    char x = 0;
    while( (ByteRead = read(fd2[0], &x, CharSize)) > 0)
    {
      if (x == ' ') write(STDOUT_FILENO, "_", sizeof("_"));
      else if (x == '\t') write(STDOUT_FILENO, "___", sizeof("___"));
      else write(STDOUT_FILENO, &x, sizeof(char));
    }
    close(fd2[0]);
  }
  return EXIT_SUCCESS;
}
