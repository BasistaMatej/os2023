#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
  int to[2];
  int from[2];

  pipe(to);
  pipe(from);

  int pid = fork();

  if (pid == 0)
  {
    char mess;
    read(from[0], &mess, 1);
    printf("%d: received ping\n", getpid());
    write(to[1], "x", 1);
  }
  else
  {
    char mess;
    write(from[1], "x", 1);
    read(to[0], &mess, 1);
    printf("%d: received pong\n", getpid());
  }

  exit(0);
}
