#include "ext2.h"

int fileSystemFormat(Disk* disk){
  SuperBlock super_block;
  GroupDescTable gdt;
  InodeBitmap inode_bitmap;
  BlockBitmap block_bitmap;
  // TODO
}