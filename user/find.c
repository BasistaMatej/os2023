#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char *my_strcat(char *destination, const char *source)
{
  char *ptr = destination + strlen(destination);

  while (*source != '\0')
  {
    *ptr++ = *source++;
  }

  *ptr = '\0';
  return destination;
}

void listFolder(int fd, char *currentParh, char *searching)
{
  struct dirent de;
  int currentFd;
  while (read(fd, &de, sizeof(de)) == sizeof(de))
  {
    if (strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0 && strcmp(de.name, "") != 0)
    {
      char buf[512];
      struct stat fileInfo;
      strcpy(buf, currentParh);
      my_strcat(buf, "/");
      my_strcat(buf, de.name);
      if ((currentFd = open(buf, 0)) < 0 || fstat(currentFd, &fileInfo) < 0)
      {
        fprintf(2, "Can not open folder (or any other problem with folder)\n");
        exit(0);
      }

      if (strcmp(de.name, searching) == 0)
      {
        printf("%s/%s\n", currentParh, de.name);
      }
      if (fileInfo.type == T_DIR)
      {
        listFolder(currentFd, buf, searching);
      }
      close(currentFd);
    }
  }
}

int main(int argc, char *argv[])
{
  struct stat fileInfo;
  int fd;
  if (argc != 3)
  {
    fprintf(2, "Invalid usage! Usage: find folder word\n");
    exit(0);
  }

  if ((fd = open(argv[1], 0)) < 0 || fstat(fd, &fileInfo) < 0)
  {
    fprintf(2, "Can not open folder (or any other problem with folder) %s\n", argv[1]);
    exit(0);
  }

  if (fileInfo.type == T_DIR)
  {
    listFolder(fd, argv[1], argv[2]);
  }
  else
  {
    fprintf(2, "%s is not folder.", argv[1]);
    exit(0);
  }
  exit(0);
}
