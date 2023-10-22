#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char* dir, char* fname) {
  char buf[512];
  char* p;
  strcpy(buf, dir);
  p = buf + strlen(buf);

  int fd;
  if ( (fd = open(buf, 0)) < 0 ) {
    printf("cno %s\n", buf);
    exit();
  }

  char forbid[5] = {'.', 0, '.', '.', 0};
  struct dirent de;
  while(read(fd, &de, sizeof(de)) == sizeof(de) && de.inum != 0) {
    if (strcmp(de.name, forbid) == 0 || strcmp(de.name, forbid + 2) == 0)
      continue;
    *p = '/';
    strcpy(p + 1, de.name);
    *(p + DIRSIZ + 1) = 0;
    
    struct stat st;
    stat(buf, &st);
    
    if (st.type == T_DIR) {
      find(buf, fname);
    } else if (st.type == T_FILE && strcmp(fname, p + 1) == 0) {
      printf("%s\n", buf);
    }
    *p = 0;
  }
  close(fd);

}

int main(int argc, char* argv[]) {
  if ( argc != 3 ) {
    printf("format file [dir] [filename]\n");
    exit();
  }
  find(argv[1], argv[2]);
  exit();
}

