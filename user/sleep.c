#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;

  if(argc != 2) {
    fprintf(0, "Invalid usage! Usage: sleep number_of_tics\n");
    exit(0);
  }
  
  i = atoi(argv[1]);
  sleep(i);
  exit(0);
}
