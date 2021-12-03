#include "ext2.h"

int format(Disk* disk) {
  assert(disk != NULL);
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;

  // 初始化超级块和组描述符
  initSuperBlock(&super_block);
  initGdt(&gdt, &super_block);
  // 将超级块和组描述符，两个位图都写入
  writeSuperBlock(disk, &super_block);
  writeGdt(disk, &super_block, &gdt);
  initInodeBitmap(disk, &super_block);
  initBlockBitmap(disk, &super_block);

  initRootDir(disk, &super_block);

  return SUCCESS;
}

int initSuperBlock(Ext2SuperBlock* super_block) {
  super_block->block_group = 0;
  super_block->blocks_count = NUMBER_OF_BLOCKS;
  super_block->blocks_per_group = NUMBER_OF_BLOCKS;
  super_block->inode_size = INODE_SIZE;
  super_block->first_data_block = SUPER_BLOCK_BASE;
  super_block->first_data_block_each_group = DATA_BLOCK_BASE;
  super_block->free_blocks_count = NUMBER_OF_BLOCKS - 1;
  super_block->free_inodes_count = NUMBER_OF_INODES - 11;
  super_block->magic = LINUX;
  super_block->first_ino = 11;
  super_block->errors = 0;
  super_block->log_block_size = 0;
  return SUCCESS;
}

int initGdt(Ext2GroupDescTable* gdt, Ext2SuperBlock* super_block) {
  Ext2GroupDesc gd;
  memset(gdt, 0, sizeof(Ext2GroupDescTable));
  memset(&gd, 0, sizeof(Ext2GroupDesc));

  gd.block_bitmap = BLOCK_BITMAP_BASE;
  gd.inode_bitmap = INODE_BITMAP_BASE;
  gd.free_blocks_count = super_block->free_blocks_count;
  gd.free_inodes_count = (super_block->free_inodes_count + 10) * 2;
  gd.used_dirs_count = 0;
  memcpy(&gdt->table[0], &gd, sizeof(Ext2GroupDesc));
  return SUCCESS;
}

int initInodeBitmap(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  for (int i = 0; i < 10; i++) setBit(block, i, 0);
  writeBlock(disk, INODE_BITMAP_BASE, block);

  return SUCCESS;
}

int initBlockBitmap(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  for (unsigned int i = 0; i < DATA_BLOCK_BASE; i++) setBit(block, i, 1);
  writeBlock(disk, BLOCK_BITMAP_BASE, block);

  return SUCCESS;
}

int initRootDir(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  // 添加根目录
  Ext2DirEntry* entry;
  entry = (Ext2DirEntry*)block;
  strcpy(entry->name, "..");
  entry->name_len = 2;
  entry->file_type = DIR_TYPE;
  entry->inode = 0;

  entry++;
  strcpy(entry->name, ".");
  entry->name_len = 1;
  entry->file_type = DIR_TYPE;
  entry->inode = 0;

  writeBlock(disk, DATA_BLOCK_BASE, block);

  Ext2SuperBlock* read_sp = (Ext2SuperBlock*)block;
  memset(block, 0, BLOCK_SIZE);
  readBlock(disk, SUPER_BLOCK_BASE, block);
  read_sp->free_blocks_count--;
  writeBlock(disk, SUPER_BLOCK_BASE, block);

  Ext2GroupDesc* gd;
  gd = (Ext2GroupDesc*)block;
  memset(block, 0, BLOCK_SIZE);
  readBlock(disk, GDT_BLOCK_BASE, block);
  gd->free_blocks_count--;
  gd->used_dirs_count = 1;
  writeBlock(disk, GDT_BLOCK_BASE, block);
  readBlock(disk, BLOCK_BITMAP_BASE, block);
  setBit(block, super_block->first_data_block_each_group, 1);
  writeBlock(disk, BLOCK_BITMAP_BASE, block);

  readBlock(disk, INODE_BITMAP_BASE, block);
  setBit(block, 0, 1);
  writeBlock(disk, INODE_BITMAP_BASE, block);

  Ext2Inode inode;
  inode.mode = 0x1FF | 0x4000;
  inode.size = 64;
  inode.blocks = 1;
  inode.block[0] = DATA_BLOCK_BASE;
  memcpy(block, &inode, sizeof(Ext2Inode));
  writeBlock(disk, INODE_TABLE_BASE, block);

  return SUCCESS;
}

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block) {
  unsigned int block_idx = SUPER_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memcpy(block, super_block, sizeof(Ext2SuperBlock));
  writeBlock(disk, block_idx, block);

  return SUCCESS;
}

int writeGdt(Disk* disk, Ext2SuperBlock* super_block, Ext2GroupDescTable* gdt) {
  unsigned int block_idx = GDT_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  memcpy(block, gdt, sizeof(Ext2GroupDescTable));
  writeBlock(disk, block_idx, block);
  return SUCCESS;
}

// int ext2Ls(Ext2FileSystem* file_system, Ext2Inode* current) {
//   Ext2Inode node;
//   Ext2DirEntry dir;
//   for (unsigned int i = 0; i< current->size; i++){

//   }
// }

int setBit(BYTE* block, int index, int value) {
  int byte = index / 8;
  int offset = index % 8;

  if (value == 0)
    block[byte] ^= (0x1 << (7 - index));
  else
    block[byte] |= (0x1 << (7 - index));
  return SUCCESS;
}

// int dirEntryPosition(int offset, UINT32 block[8]) {
//   int dir_block = offset / 512;     // 存储目录需要的块数
//   int block_offset = offset % 512;  // 块内偏移字节数
//   int a;
//   if (dir_block < 6) {
//     // 前6个直接索引
//     return DATA_BLOCK_BASE + block[dir_block];
//   } else {
//     // 间接索引
//     dir_block = dir_block - 6;
//     if (dir_block < 128) {
//       // 一个块512字节，一个int为4个字节，一级索引有512/4=128个
//       int a;
//       fseek(fp,
//             DATA_BEGIN_BLOCK * BLOCK_SIZE + i_block[6] * BLOCK_SIZE +
//                 dir_blocks * 4,
//             SEEK_SET);
//       fread(&a, sizeof(int), 1, fp);
//       BYTE block[BLOCK_SIZE];
//       memset(block, 0, BLOCK_SIZE);
//       disk
//       return DATA_BEGIN_BLOCK * BLOCK_SIZE + a * BLOCK_SIZE + block_offset;
//     } else {
//       // 二级索引
//       dir_blocks = dir_blocks - 128;
//       fseek(fp,
//             DATA_BEGIN_BLOCK * BLOCK_SIZE + i_block[7] * BLOCK_SIZE / 128 *
//             4, SEEK_SET);
//       fread(&a, sizeof(int), 1, fp);
//       fseek(
//           fp,
//           DATA_BEGIN_BLOCK * BLOCK_SIZE + a * BLOCK_SIZE + dir_blocks % 128 *
//           4, SEEK_SET);
//       fread(&a, sizeof(int), 1, fp);
//       return DATA_BEGIN_BLOCK * BLOCK_SIZE + a * BLOCK_SIZE + block_offset;
//     }
//     fclose(fp);
//   }
//   return 0;
// }

int writeBlock(Disk* disk, unsigned int block_idx, void* block) {
  disk->write_disk(disk, block_idx, block);
  return SUCCESS;
}

int readBlock(Disk* disk, unsigned int block_idx, void* block) {
  disk->read_disk(disk, block_idx, block);
  return SUCCESS;
}