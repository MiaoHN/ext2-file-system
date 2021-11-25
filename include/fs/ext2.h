/**
 * @file ext2.h
 * @author MiaoHN (582418227@qq.com)
 * @brief 模拟 EXT2 文件系统
 * @version 0.1
 * @date 2021-11-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __EXT2_H__
#define __EXT2_H__

#include <string.h>

#include "common.h"
#include "disk.h"
#include "type.h"

/******************************** DEFINE **************************************/

// Size

#define DISK_SIZE 2097152
#define SECTOR_SIZE 512
#define BLOCK_SIZE (SECTOR_SIZE * SECTORS_PER_BLOCK)
#define INODE_SIZE 128

#define NUMBER_OF_BLOCKS 232
#define NUMBER_OF_GROUPS 1
#define NUMBER_OF_INODES 3231

// Per

#define INODES_PER_GROUP (NUMBER_OF_INODES / NUMBER_OF_GROUPS)
#define INODES_PER_BLOCK (BLOCK_SIZE / INODE_SIZE)
#define BLOCKS_PER_GROUP 1
#define SECTORS_PER_BLOCK 8

#define INODE_BLOCKS 15

#define NAME_LENGTH 255

#define INODE_SIZE 128

#define NUMBER_OF_GROUPS 1
#define NUMBER_OF_INODES (NUMBER_OF_BLOCKS / 2)

#define INODES_PER_GROUP (NUMBER_OF_INODES / NUMBER_OF_GROUPS)
#define BLOCKS_PER_GROUP (NUMBER_OF_BLOCKS / NUMBER_OF_GROUPS)

#define BOOT_BASE 0
#define SUPER_BLOCK_BASE (BOOT_BASE + 0)
#define GDT_BASE (SUPER_BLOCK_BASE + 1)
#define INODE_BITMAP_BASE (GDT_BASE + 1)
#define BLOCK_BITMAP_BASE (INODE_BITMAP_BASE + 1)
#define INODE_TABLE_BASE (BLOCK_BITMAP_BASE + 1)
#define DATA_BLOCK_BASE (INODE_TABLE_BASE + 1)

#define EXT2_SUCCESS 0
#define EXT2_ERROR 1

#define EXT2_VALID_FS 0x0001
#define EXT2_ERROR_FS 0x0002

// 超级块
typedef struct Ext2SuperBlock {
  UINT32 inodes_count;       // 索引结点的总数
  UINT32 blocks_count;       // 文件系统块的总数
  UINT32 r_blocks_count;     // 为超级用户保留的块数

  UINT32 free_blocks_count;  // 空闲块总数
  UINT32 free_inodes_count;  // 空闲索引结点总数

  UINT32 first_data_block;   // 文件系统中第一个数据块
  UINT32 log_block_size;     // 用于计算逻辑块的大小
  UINT32 log_fragment_size;  // 用于计算片的大小

  UINT32 blocks_per_group;     // 每个组的块个数
  UINT32 fragments_per_group;  // 每个组的片个数
  UINT32 inodes_per_group;     // 每个组的索引结点数

  UINT32 mtime;            // 文件系统的安装时间
  UINT32 wtime;            // 最后一次对超级块进行写的时间
  UINT16 mount_count;      // 安装计数
  UINT16 max_mount_count;  // 最大可安装计数
  UINT16 magic_signature;  // 用于确定文件系统版本的标志 (ext2 -- 0xEF53)
  UINT16 state;            // 文件系统状态
  UINT16 errors;           // 当检测到错误时如何处理
  UINT16 minor_rev_level;  // 次版本号

  UINT32 last_check;  // 最后一次检测文件系统状态的时间
  UINT32 check_interval;  // 两次对文件系统状态进行检测的最大可能时间间隔
  UINT32 creator_os;  // 适用的操作系统
  UINT32 rev_level;  // 版本号，以此识别是否支持是否支持某些功能

  UINT16 def_fesuid;     // 保留块的默认用户标识 UID
  UINT16 def_fesgid;     // 保留块的默认用户组标识 GID
  UINT32 first_ino;      // 第一个非保留的索引结点号
  UINT16 inode_size;     // 索引结点结构的大小
  UINT16 block_group;    // 本 SuperBlock 所在的块组号
  UINT32 reserved[230];  // 保留
} Ext2SuperBlock;


// 组描述符
typedef struct Ext2GroupDesc {
  UINT32 block_bitmap;       // 指向该组中块位图所在块的指针
  UINT32 inode_bitmap;       // 指向该组中块结点位图所在块的指针
  UINT32 inode_table;        // 指向该组中结点的首块的指针
  UINT16 free_blocks_count;  // 本组空闲块的个数
  UINT16 free_inodes_count;  // 本组空闲索引结点的个数
  UINT16 used_dirs_count;    // 本组分配给目录的结点数
  UINT16 pad;                // 填充
  UINT32 reserved;           // 保留
} Ext2GroupDesc;

// 索引结点
typedef struct Ext2Inode {
  UINT16 mode;                  // 文件类型及访问权限
  UINT16 uid;                   // 文件拥有者的标识号 UID
  UINT32 size;                  // 文件大小(字节)
  UINT32 atime;                 // 最后一次访问时间
  UINT32 ctime;                 // 创建时间
  UINT32 mtime;                 // 该文件内容最后修改时间
  UINT32 dtime;                 // 文件删除时间
  UINT16 gid;                   // 文件的用户组的组号
  UINT16 links_count;           // 文件的链接计数
  UINT32 blocks;                // 文件的数据块个数(以512字节计)
  UINT32 flags;                 // 打开文件的方式
  UINT32 block[EXT2_N_BLOCKS];  // 指向数据块的指针数组
  UINT32 generation;            // 文件的版本号(用于 NFS)
  UINT32 file_acl;              // 文件访问控制表( ACL 已不再使用)
  UINT32 dir_acl;               // 目录访问控制表( ACL 已不再使用)
  BYTE frag;                    // 每块中的片数
  BYTE fsize;                   // 片的大小
  UINT32 reserved;              // 保留
} Ext2Inode;

// 目录块
typedef struct Ext2DirEntry {
  UINT32 inode;         // 索引结点号
  UINT16 rec_len;       // 目录项长度
  BYTE name_len;        // 文件名长度
  BYTE file_type;       // 文件类型(1: 普通文件 2: 目录 ...)
  char name[NAME_LEN];  // 文件名
} Ext2DirEntry;

typedef struct Ext2GroupDescTable {
  Ext2GroupDesc table[NUMBER_OF_GROUPS];
} Ext2GroupDescTable;

typedef struct Ext2InodeTable {
  Ext2Inode inode[INODES_PER_GROUP];
} Ext2InodeTable;

typedef struct Bitmap {
  BYTE bitmap[BLOCK_SIZE];
} Bitmap;

typedef Bitmap Ext2InodeBitmap;
typedef Bitmap Ext2BlockBitmap;

/********************************* INIT ***************************************/

int format(Disk* disk);
int initSuperBlock(Ext2SuperBlock* super_block);
int initGdt(Ext2SuperBlock* super_block, Ext2GroupDescTable* gdt);
int initInodeBitmap(Disk* disk, int group);
int initBlockBitmap(Disk* disk, int group);
int initRootDir(Disk* disk, Ext2SuperBlock* super_block);

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block, int group);

/********************************* UTILS **************************************/

int setBit(BYTE* block, int index, int value);

// 向 disk 中的一个 block 写入数据
int writeBlock(Disk* disk, int idx, BYTE* block);

// 从 disk 中的一个 block 读出数据
int readBlock(Disk* disk, int idx, BYTE* block);


/**
 * @brief 文件系统
 *
 */
typedef struct FileSystem {
  SuperBlock super_block;  // 超级块
  GroupDesc group_desc;    // 组描述符
  Disk* disk;              // 硬盘
} FileSystem;

/**
 * @brief 文件入口的具体位置
 *
 */
typedef struct DirEntryLocation {
  UINT32 group;
  UINT32 block;
  UINT32 offset;
} DirEntryLocation;

/**
 * @brief 保存当前文件系统及工作区信息
 *
 */
typedef struct Node {
  FileSystem* file_system;
  DirEntry entry;
  DirEntryLocation location;
} Node;

/********************************* INIT ***************************************/

int fileSystemFormat(Disk* disk);
int fileSystemMount(FileSystem* file_system, Disk* disk);
int fileSystemUMount(FileSystem* file_system, Disk* disk);

int fillSuperBlock(SuperBlock* super_block);
int fillGdt(GroupDescTable* gdt);

// 从 disk 中读取文件系统的信息并初始化
int initFileSystem(FileSystem* file_system);

int initSuperBlock(Disk* disk, SuperBlock* super_block, UINT32 group_number);
int initGdt(Disk* disk, GroupDescTable* gdt, UINT32 group_number);
int initBlockBitmap(Disk* disk, UINT32 group_number);
int initInodeBitmap(Disk* disk, UINT32 group_number);

/***************************** SETTER GETTER **********************************/

int setBit(BYTE* bitmap, SECTOR index, int val);
int getBit(BYTE* bitmap, SECTOR index);

int readSuperBlock(FileSystem* file_system, Node* node);

int getInode(FileSystem* file_system, UINT32 inode_idx, Inode* inode);
int getSuperBlock(FileSystem* file_system, UINT32 group_number,
                  SuperBlock* super_block);

/********************************* UTILS **************************************/

int blockWrite(Disk* disk, SECTOR block, DATA data);
int blockRead(Disk* disk, SECTOR block, DATA data);

#endif  // __EXT2_H__