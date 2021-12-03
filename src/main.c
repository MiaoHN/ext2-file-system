#include <stdio.h>

#include "disk.h"
#include "ext2.h"

int main() {
  Ext2FileSystem file_system;
  file_system.disk = malloc(sizeof(Disk));
  makeDisk(file_system.disk, "my_disk.dsk", NUMBER_OF_BLOCKS);

  format(file_system.disk);
  Ext2Inode current;
  getRootInode(&file_system, &current);

  ext2Ls(&file_system, &current);

  return 0;
}