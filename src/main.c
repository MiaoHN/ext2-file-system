#include <stdio.h>

#include "disk.h"
#include "ext2.h"
#include "shell.h"

int main() {
  // if (freopen("script.txt", "r", stdin) ==
  //     NULL)  //将标准输入流重定向至stdin.txt流
  // {
  //   fprintf(stderr, "打开文件失败！");
  //   exit(-1);
  // }
  shellStart();
  return 0;
}