#include <stdio.h>

#include "disk.h"
#include "ext2.h"

int main() {
  Disk disk;
  // makeDisk(&disk , "./my_disk.dsk", NUMBER_OF_BLOCKS);
  loadDisk(&disk, "./my_disk.dsk");

  format(&disk);

  printf("Disk Info:\n");
  printf("    block size: \n");
  return 0;
}