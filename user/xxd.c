#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int main(int argc, char* argv[]) {
  int fd;
  if (argc != 2) {
    printf("1 arg\n");
    exit();
  }
  if ( (fd = open(argv[1], 0)) < 0) {
    printf("cno %s\n", argv[1]);
    exit();
  }
  #define I 1000
  char buf[I + 1];
  char buf2[I + 1];
  
  while(1) {
    memset(buf, 0, I + 1);
    memset(buf2, 0, I + 1);
    
    int qq = read(fd, buf, I);
    for (char* p = buf, *q = buf2; p < buf + qq; p++, q++) {
      *q = *p;
      if ( (*p < 65 || *p > 90) && (*p < 97 || *p > 122) && *p != '.') {
        *p = 32;
      }
    }
    printf("%s", buf);
    for (char* q = buf2; q < buf2 + qq; q++) {
      printf("%x%x", *(uint8*)q/16, *(uint8*)q % 16);
    }    
    printf("\n\n");
    if (qq != I) break;
  }

  close(fd);
}
