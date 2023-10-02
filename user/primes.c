#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int isPrime(int a)
{
  for (int i = 2; i <= (a / 2); i++)
  {
    if (a % i == 0)
    {
      return 0;
    }
  }
  return 1;
}

int primeFork(int startFd)
{
  int p, n;
  int write_fd[2];
  int no_child = 1;
  pipe(write_fd);

  read(startFd, &p, sizeof(int));
  printf("prime %d\n", p);
  while (read(startFd, &n, sizeof(int)) == sizeof(int))
  {
    if ((n % p) != 0)
    {
      if (no_child)
      {
        no_child = 0;
        int pid = fork();
        if (pid == 0)
        {
          // Child / read
          close(write_fd[1]);
          primeFork(write_fd[0]);
          break;
        }
        else if (pid < 0)
        {
          fprintf(2, "error: fork failed");
        }
      }
      write(write_fd[1], &n, sizeof(int));
    }
  }
  close(write_fd[1]);
  return 1;
}

int main(int argc, char *argv[])
{
  int p[2];

  pipe(p);

  int pid = fork();

  if (pid == 0)
  {
    // Dieta/Read strana
    close(p[1]);
    primeFork(p[0]);
  }
  else if (pid > 0)
  {
    // Parent/ write strana
    for (int i = 2; i <= 35; i++)
    {
      write(p[1], &i, sizeof(int));
    }
  }
  else
  {
    fprintf(2, "Fork error!");
    exit(0);
  }

  exit(0);
}
