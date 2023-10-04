#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int child(int start_fd)
{
  int number, nextNumber, pid, status;
  int p[2];
  int readSide = 0;

  pipe(p);
  read(start_fd, &number, sizeof(int));
  fprintf(2, "prime: %d\n", number);
  while (read(start_fd, &nextNumber, sizeof(int)) == sizeof(int))
  {
    // fprintf(2, "prime: %d\n", nextNumber);

    if ((nextNumber % number) != 0)
    {
      if (readSide == 0)
      {
        readSide = 1;
        pid = fork();

        if (pid == 0)
        {
          // Left side
          close(p[1]);
          child(p[0]);
        }
        else if (pid < 0)
        {
          fprintf(2, "Fork error.");
          exit(0);
        }
      }
      write(p[1], &nextNumber, sizeof(int));
    }
  }
  close(p[1]);
  wait(&status);
  return 1;
}

int main(int argc, char *argv[])
{
  int status, p[2];

  pipe(p);

  int pid = fork();

  if (pid == 0)
  {
    // Right side
    close(p[1]);
    child(p[0]);
  }
  else if (pid > 0)
  {
    // Left side
    for (int i = 2; i <= 35; i++)
    {
      write(p[1], &i, sizeof(int));
    }
    close(p[1]);
  }
  else
  {
    fprintf(2, "Fork error.");
    exit(0);
  }
  wait(&status);
  exit(0);
}
