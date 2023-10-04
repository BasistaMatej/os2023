#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
  if(argc != 3) 
  {
     fprintf(2, "Invalid usage!\n");
     exit(1);
  }
  
  int src, des, n;
  char buf[512];
  src = open(argv[1], O_RDONLY);
  des = open(argv[2], O_WRONLY|O_CREATE);
  
  if(src < 0 || des < 0) 
  {
     fprintf(2,"Error read file or files");
     exit(1);
  }
  
  while((n = read(src, buf, sizeof(buf))) > 0) 
  {
     if(write(des, buf, n) != n) 
     {
        fprintf(2,"Write error\n");
     	exit(1);
     }
  }
  
  close(src);
  close(des);
  

  exit(0);
}
