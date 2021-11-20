#include <stdio.h>

#include "disk.h"
#include "common.h"
#include "shell.h"

int main(int argc, char const* argv[]) {
  // shellLoop();

  Disk disk;

  diskInit(&disk, 4096, 512, "./my_disk.dsk");

  char data1[BLOCK_SIZE] = "Hello World!";
  disk.write_sector(&disk, 24, data1);

  char data2[BLOCK_SIZE] = "Have a good day!";
  disk.write_sector(&disk, 30, data2);

  char ptr2[BLOCK_SIZE];
  char ptr1[BLOCK_SIZE];

  disk.read_sector(&disk, 24, ptr1);
  disk.read_sector(&disk, 30, ptr2);
  

  printf("%s\n", ptr1);
  printf("%s\n", ptr2);

  return 0;
}
