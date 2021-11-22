/**
 * @file main.c
 * @author MiaoHN (582418227@qq.com)
 * @brief 程序入口
 * @version 0.1
 * @date 2021-11-21
 *
 * @copyright Copyright (c) 2021
 *
 */

#include <stdio.h>

#include "common.h"
#include "disk.h"
#include "ext2.h"
#include "shell.h"

int main(int argc, char const* argv[]) {
  shellLoop();

  return 0;
}
