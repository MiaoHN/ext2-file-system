#include "ext2.h"

int fileSystemFormat(Disk* disk) {
  SuperBlock super_block;
  GroupDescTable gdt;
  InodeBitmap inode_bitmap;
  BlockBitmap block_bitmap;
  InodeTable inode_table;

  if (fillSuperBlock(&super_block) != EXT2_SUCCESS) return EXT2_ERROR;

  for (int i = 0; i < NUMBER_OF_GROUPS; i++) {
    super_block.block_group_number = i;
    initSuperBlock(disk, &super_block, i);
  }
}

/******************************* INIT *********************************/

int initSuperBlock(Disk* disk, SuperBlock* super_block, UINT32 group_number) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, sizeof(block));
  memcpy(block, super_block, sizeof(block));
  blockWrite(disk, SUPER_BLOCK_BASE + (group_number * BLOCKS_PER_GROUP), block);

  return EXT2_SUCCESS;
}

/******************************* FILL *********************************/

int fillSuperBlock(SuperBlock* super_block) {
  memset(super_block, 0, sizeof(SuperBlock));
  super_block->inodes_count = NUMBER_OF_INODES;
  super_block->first_metablock_group = 0;  // ?
  super_block->blocks_count = NUMBER_OF_BLOCKS;
  super_block->reserved_blocks_count = super_block->blocks_count / 100 * 5;
  super_block->free_blocks_count =
      NUMBER_OF_BLOCKS - (super_block->first_metablock_group) - 1;
  super_block->free_inodes_count = NUMBER_OF_INODES - 11;
  super_block->first_data_block = SUPER_BLOCK_BASE;
  super_block->log_block_size = 0;
  super_block->log_fragment_size = 0;
  super_block->blocks_per_group = BLOCKS_PER_GROUP;
  super_block->fragments_per_group = 0;
  super_block->inodes_per_group = NUMBER_OF_INODES / NUMBER_OF_GROUPS;
  super_block->magic_signature = 0xEF53;
  super_block->errors = 0;
  super_block->first_non_reserved_inode = 11;
  super_block->inode_structure_size = 128;

  return EXT2_SUCCESS;
}

/***************************** SETTER GETTER **********************************/

/********************************** UTILS *************************************/

int blockWrite(Disk* disk, SECTOR block, DATA data) {
  int result;
  int sectorNumber = block * SECTORS_PER_BLOCK;

  for (int i = 0; i < SECTORS_PER_BLOCK; i++) {
    result =
        disk->write_sector(disk, sectorNumber + i, data + (i * SECTOR_SIZE));
  }
  return result;
}

int blockRead(Disk* disk, SECTOR block, DATA data) {
  int result;
  int sectorNumber = block * SECTORS_PER_BLOCK;

  for (int i = 0; i < SECTORS_PER_BLOCK; i++) {
    result =
        disk->read_sector(disk, sectorNumber + i, data + (i * SECTOR_SIZE));
  }

  return result;
}