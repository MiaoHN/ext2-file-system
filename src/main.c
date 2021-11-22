#include <stdio.h>

#include "common.h"
#include "disk.h"
#include "shell.h"
#include "debug.h"

int main(int argc, char const* argv[]) {
  // shellLoop();

  Disk disk;

  diskInit(&disk, NUMBERS_OF_SECTOR, BYTES_PER_SECTOR, "./my_disk.dsk");

  BYTE data1[SECTOR_SIZE] = "Hello World!";
  disk.write_sector(&disk, 22, data1);

  BYTE ptr1[SECTOR_SIZE];

  disk.read_sector(&disk, 22, ptr1);

  dumpDisk(&disk, BYTES_PER_SECTOR * 22, 13);

  return 0;
}
