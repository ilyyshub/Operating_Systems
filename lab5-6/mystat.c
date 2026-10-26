#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *
typename(int type)
{
  switch(type){
  case T_DIR:
    return "directory";
  case T_FILE:
    return "file";
  case T_DEVICE:
    return "device";
  default:
    return "?";
  }
}

int
main(int argc, char *argv[])
{
  struct stat st;

  if(argc < 2){
    fprintf(2, "usage: mystat <path> [path ...]\n");
    exit(1);
  }

  for(int i = 1; i < argc; i++){
    if(stat(argv[i], &st) < 0){
      fprintf(2, "mystat: cannot stat %s\n", argv[i]);
      continue;
    }

    printf("%s: %s, inode %u, links %d, size %lu bytes\n",
           argv[i], typename(st.type), st.ino, st.nlink,
           (unsigned long)st.size);
  }

  exit(0);
}
