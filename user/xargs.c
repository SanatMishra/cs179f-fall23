#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("format xargs [command]\n");
    exit();
  }
  #define I 100
  char* argve[I + 1];
  memset(argve, 0, I + 1);

  int argce = 0;
  for (argce = 0; argce + 1 < argc; argce++)
    argve[argce] = argv[argce + 1];

  #define J 100
  char c[J];
  memset(c, 0, J);
  int newarg = 1;
  int qq = read(0, c, J);
  for (int i = 0; i < qq; i++) {
    if (c[i] == ' ' || c[i] == '\n' || c[i] == '\r') {
      c[i] = 0;
      newarg = 1;
    } else if (newarg) {
      argve[argce++] = c + i;
      newarg = 0;
    }
  }

  exec(argv[1], argve);
  exit();
}

