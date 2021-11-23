#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "disk.h"
#include "ext2.h"
#include "shell.h"

int main(int argc, char const* argv[]) {
  // shellLoop();

  Disk disk;

  diskInit(&disk, NUMBERS_OF_SECTOR, BYTES_PER_SECTOR, "./my_disk.dsk");

  BYTE data1[BLOCK_SIZE] = "Hello World!";
  writeBlock(&disk, 4, data1);
  BYTE ptr1[BLOCK_SIZE];
  readBlock(&disk, 4, ptr1);

  dumpDisk(&disk, BLOCK_SIZE * 4, 99);
  printf("readBlock: %s", ptr1);

  return 0;
}
