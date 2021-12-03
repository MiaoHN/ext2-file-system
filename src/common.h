#ifndef __COMMON_H__
#define __COMMON_H__

#define SUCCESS 0
#define FAILURE -1

#include <assert.h>

#define EXT2_N_BLOCKS 8

#define DIR_NAME_LEN 15


#define LINUX 0xEF53

#define DISK_BOOT_BASE 0
#define SUPER_BLOCK_BASE (DISK_BOOT_BASE + 0)
#define GDT_BLOCK_BASE (SUPER_BLOCK_BASE + 1)
#define INODE_BITMAP_BASE (GDT_BLOCK_BASE + 1)
#define BLOCK_BITMAP_BASE (INODE_BITMAP_BASE + 1)
#define INODE_TABLE_BASE (BLOCK_BITMAP_BASE + 1)
#define DATA_BLOCK_BASE (INODE_TABLE_BASE + 20)

typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char BYTE;

#define DIR_TYPE 2
#define FILE_TYPE 1

#define SECTORS_PRE_BLOCK 1
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)

#define SECTOR_SIZE 512
#define BLOCK_SIZE (SECTOR_SIZE * SECTORS_PRE_BLOCK)
#define INODE_SIZE 128
#define DIR_SIZE 32

#define NUMBER_OF_BLOCKS 4096
#define NUMBER_OF_INODES (NUMBER_OF_BLOCKS / 4)

#endif  // __COMMON_H__