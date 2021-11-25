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
#include "debug.h"
#include "disk.h"
#include "ext2.h"
#include "shell.h"

int main(int argc, char const* argv[]) {
  // shellLoop();

  Disk disk;

  diskInit(&disk, NUMBER_OF_SECTORS, BYTES_PER_SECTOR, "./my_disk.dsk");

  format(&disk);

  dumpDisk(&disk, INODE_BITMAP_BASE * BLOCK_SIZE, 100);

  // BYTE data1[BLOCK_SIZE] = "Hello World!";
  // writeBlock(&disk, 4, data1);
  // BYTE ptr1[BLOCK_SIZE];
  // readBlock(&disk, 4, ptr1);

  // dumpDisk(&disk, BLOCK_SIZE * 4, 99);
  // printf("readBlock: %s", ptr1);

  return 0;
}
