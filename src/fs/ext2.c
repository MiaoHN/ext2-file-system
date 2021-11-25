#include "ext2.h"

/********************************* INIT ***************************************/

int format(Disk* disk) {
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;

  initSuperBlock(&super_block);
  initGdt(&super_block, &gdt);

  for (int i = 0; i < NUMBER_OF_GROUPS; i++) {
    super_block.block_group = i;
    writeSuperBlock(disk, &super_block, i);
    initInodeBitmap(disk, i);
    initBlockBitmap(disk, i);
  }

  initRootDir(disk, &super_block);

  return EXT2_SUCCESS;
}

int initSuperBlock(Ext2SuperBlock* super_block) {
  memset(super_block, 0, sizeof(Ext2SuperBlock));
  super_block->block_group = 0;
  super_block->blocks_count = NUMBER_OF_BLOCKS;
  super_block->blocks_per_group = BLOCKS_PER_GROUP;
  super_block->inode_size = INODE_SIZE;
  super_block->first_data_block = SUPER_BLOCK_BASE;
  super_block->free_blocks_count = NUMBER_OF_BLOCKS - 1;
  super_block->free_inodes_count = NUMBER_OF_INODES - 11;
  super_block->magic = 0xEF53;
  super_block->first_ino = 11;
  super_block->errors = 0;
  super_block->log_block_size = 0;
  return EXT2_SUCCESS;
}

int initGdt(Ext2SuperBlock* super_block, Ext2GroupDescTable* gdt) {
  Ext2GroupDesc gd;
  memset(gdt, 0, sizeof(Ext2GroupDescTable));
  memset(&gd, 0, sizeof(Ext2GroupDesc));

  for (int i = 0; i < NUMBER_OF_GROUPS; i++) {
    int offset = i * BLOCKS_PER_GROUP;
    gd.block_bitmap = BLOCK_BITMAP_BASE + offset;
    gd.inode_bitmap = INODE_BITMAP_BASE + offset;
    gd.free_blocks_count = super_block->free_blocks_count / NUMBER_OF_GROUPS;
    gd.free_inodes_count =
        (super_block->free_inodes_count + 10) / NUMBER_OF_GROUPS * 2;
    gd.used_dirs_count = 0;
    memcpy(&gdt->table[i], &gd, sizeof(Ext2GroupDesc));
  }
  return EXT2_SUCCESS;
}

int initBlockBitmap(Disk* disk, int group) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, sizeof(block));

  for (int i = 0; i < DATA_BLOCK_BASE; i++) setBit(block, i, 1);
  writeBlock(disk, BLOCK_BITMAP_BASE + (group * BLOCKS_PER_GROUP), block);
  return EXT2_SUCCESS;
}

int initInodeBitmap(Disk* disk, int group) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, sizeof(block));

  for (int i = 0; i < 10; i++) setBit(block, i, 1);
  writeBlock(disk, INODE_BITMAP_BASE + (group * BLOCKS_PER_GROUP), block);
  return EXT2_SUCCESS;
}

int initRootDir(Disk* disk, Ext2SuperBlock* super_block) {
  
}

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block, int group) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, sizeof(block));
  memcpy(block, super_block, sizeof(block));
  writeBlock(disk, SUPER_BLOCK_BASE + group * BLOCKS_PER_GROUP, block);
  return EXT2_SUCCESS;
}

/********************************* UTILS **************************************/

int setBit(BYTE* block, int index, int value) {
  int byte = index / 8;
  int offset = index % 8;

  if (value == 0)
    block[byte] ^= (0x1 << (7 - index));
  else
    block[byte] |= (0x1 << (7 - index));
  return EXT2_SUCCESS;
}

int writeBlock(Disk* disk, int idx, BYTE* block) {
  for (int i = 0; i < SECTORS_PER_BLOCK; i++) {
    disk->write_sector(disk, idx * SECTORS_PER_BLOCK + i,
                       block + i * BYTES_PER_SECTOR);
  }
}

int readBlock(Disk* disk, int idx, BYTE* block) {
  for (int i = 0; i < SECTORS_PER_BLOCK; i++) {
    disk->read_sector(disk, idx * SECTORS_PER_BLOCK + i,
                      block + i * BYTES_PER_SECTOR);
  }
}