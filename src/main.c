#include <stdio.h>

#include "disk.h"
#include "ext2.h"
#include "shell.h"

int main() {
#if 0
  if (freopen("script.txt", "r", stdin) == NULL) {
    fprintf(stderr, "打开文件失败！");
    exit(-1);
  }
#endif
  shellStart();
  return 0;
}
