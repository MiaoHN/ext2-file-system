#include <stdio.h>

#include "disk.h"

int main() {
  Disk disk;
  // makeDisk(&disk , "./my_disk.dsk", 4096, 512, 1);
  loadDisk(&disk, "./my_disk.dsk");

  printf("Disk Info:\n");
  printf("    sector size: %d\n", disk.disk_info.sector_size);
  printf("    block size: %d\n", disk.disk_info.disk_size);
  return 0;
}