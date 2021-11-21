#include "ext2.h"

int fileSystemFormat(Disk* disk) {
  SuperBlock super_block;
  GroupDescTable gdt;
  InodeBitmap inode_bitmap;
  BlockBitmap block_bitmap;
  InodeTable inode_table;
  int i;
}

int fillSuperBlock(SuperBlock* super_block) {
  memset(super_block, 0, sizeof(SuperBlock));
  super_block->inodes_count = NUMBER_OF_INODES;
  super_block->first_data_block =
}