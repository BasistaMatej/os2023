#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p[2];
  
  p[0] = getpid();
  p[1] = fork();
  pipe(p);
  fprintf(0, "%d" ,getpid());

  exit(0);
}
