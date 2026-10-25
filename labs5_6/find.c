#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

static void
find(char *path, char *target)
{
  char buf[512];
  char name[DIRSIZ + 1];
  char *base;
  int fd;
  uint len;
  struct stat st;
  struct dirent de;

  if(stat(path, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    return;
  }

  base = path;
  for(char *p = path; *p; p++){
    if(*p == '/')
      base = p + 1;
  }

  if(strcmp(base, target) == 0)
    printf("%s\n", path);

  if(st.type != T_DIR)
    return;

  fd = open(path, 0);
  if(fd < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  len = strlen(path);
  if(len + 1 + DIRSIZ + 1 > sizeof(buf)){
    fprintf(2, "find: path too long: %s\n", path);
    close(fd);
    return;
  }

  strcpy(buf, path);
  buf[len] = '/';

  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;

    memmove(name, de.name, DIRSIZ);
    name[DIRSIZ] = 0;

    if(strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
      continue;

    strcpy(buf + len + 1, name);
    find(buf, target);
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "usage: find <dir> <name>\n");
    exit(1);
  }

  find(argv[1], argv[2]);
  exit(0);
}
