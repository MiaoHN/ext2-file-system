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

// Status

#define EXT2_SUCCESS 0
#define EXT2_ERROR -1

// Base

#define BOOT_BLOCK_BASE 0
#define SUPER_BLOCK_BASE (BOOT_BLOCK_BASE + 0)
#define GDT_BASE (SUPER_BLOCK_BASE + 1)
#define DATA_BLOCK_BITMAP_BASE (GDT_BASE + 1)
#define INODE_BITMAP_BASE (DATA_BLOCK_BITMAP_BASE + 1)
#define INODE_TABLE_BASE (INODE_BITMAP_BASE + 1)
#define DATA_BLOCK_BASE (INODE_TABLE_BASE + 1)

/**
 * @brief 文件类型
 *
 */
enum FILE_TYPE {
  TYPE_UNKNOWN = 0,  // 未知文件类型
  TYPE_REG_FILE = 1,
  TYPE_DIR = 2,  // 目录
  TYPE_CHRDEV = 3,
  TYPE_BLKDEV = 4,
  TYPE_FIFO = 5,
  TYPE_SOCK = 6,
  TYPE_SYMLINK = 7,
  TYPE_MAX
};

/******************************** STRUCTURE ***********************************/

/**
 * @brief 超级块
 *
 */
typedef struct SuperBlock {
  UINT32 inodes_count;  // 索引结点的总数
  UINT32 blocks_count;  // 文件系统块的总数
  UINT32 reserved_blocks_count;  // 为超级用户保留的块数，一般占内存的 5%
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
  UINT16 UID_for_reserved_block;  // 保留块的默认用户标识 UID
  UINT16 GID_for_reserved_block;  // 保留块的默认用户组标识 GID

  UINT32 first_non_reserved_inode;  // 第一个非保留的索引结点号
  UINT16 inode_structure_size;      // 索引结点结构的大小
  UINT16 block_group_number;        // 本 SuperBlock 所在的块组号
  UINT32 compatible_feature;
  UINT32 incompatible_feature;
  UINT32 read_only_feature;
  UINT32 UUID[4];
  BYTE volume_name[16];           // 文件系统名称
  UINT32 last_mounted_path[16];   // 上次挂载位置
  UINT32 algorithm_usage_bitmap;  // For compression

  UINT8 preallocated_blocks_count;
  UINT8 preallocated_dir_blocks_count;
  BYTE padding[2];

  UINT32 journal_uuid[4];
  UINT32 journal_inode_number;
  UINT32 journal_device;
  UINT32 orphan_inode_list;  // 删除 inode 链表的起始处
  UINT32 hash_seed[4];
  UINT8 default_hash_version;  // 默认为 signed_directory_hash
  BYTE reserved_char_pad;
  BYTE reserved_word_pad[2];
  UINT32 default_mount_option;   // 默认为 user_xattr acl
  UINT32 first_metablock_group;  // 第一个元块组集

  BYTE reserved[760];  // 保留
} SuperBlock;

/**
 * @brief 组描述符
 *
 */
typedef struct GroupDesc {
  UINT32 block_bitmap;       // 指向该组中块位图所在块的指针
  UINT32 inode_bitmap;       // 指向该组中块结点位图所在块的指针
  UINT32 inode_table;        // 指向该组中结点的首块的指针
  UINT16 free_blocks_count;  // 本组空闲块的个数
  UINT16 free_inodes_count;  // 本组空闲索引结点的个数
  UINT16 used_dirs_count;    // 本组分配给目录的结点数
  UINT16 pad;                // 填充
  UINT32 reserved;           // 保留
} GroupDesc;

/**
 * @brief gdt
 *
 */
typedef struct GroupDescTable {
  GroupDesc group_desc[NUMBER_OF_GROUPS];
} GroupDescTable;

/**
 * @brief 索引结点
 *
 */
typedef struct Inode {
  UINT16 mode;                 // 文件类型及访问权限
  UINT16 uid;                  // 文件拥有者的标识号 UID
  UINT32 size;                 // 文件大小(字节)
  UINT32 atime;                // 最后一次访问时间
  UINT32 ctime;                // 创建时间
  UINT32 mtime;                // 该文件内容最后修改时间
  UINT32 dtime;                // 文件删除时间
  UINT16 gid;                  // 文件的用户组的组号
  UINT16 links_count;          // 文件的链接计数
  UINT32 blocks;               // 文件的数据块个数(以512字节计)
  UINT32 flags;                // 打开文件的方式
  UINT32 block[INODE_BLOCKS];  // 指向数据块的指针数组
  UINT32 generation;           // 文件的版本号(用于 NFS)
  UINT32 file_acl;             // 文件访问控制表( ACL 已不再使用)
  UINT32 dir_acl;              // 目录访问控制表( ACL 已不再使用)
  BYTE frag;                   // 每块中的片数
  BYTE fsize;                  // 片的大小
  UINT32 reserved;             // 保留
} Inode;

typedef struct BlockBitmap {
  BYTE block_bitmap[BLOCK_SIZE];
} BlockBitmap;

typedef struct InodeBitmap {
  BYTE inode_bitmap[BLOCK_SIZE];
} InodeBitmap;

typedef struct InodeTable {
  Inode inode_table[INODES_PER_GROUP];
} InodeTable;

/**
 * @brief 目录块
 *
 */
typedef struct DirEntry {
  UINT32 inode;            // 索引结点号
  UINT16 rec_len;          // 目录项长度
  BYTE name_len;           // 文件名长度
  BYTE file_type;          // 文件类型(1: 普通文件 2: 目录 ...)
  char name[NAME_LENGTH];  // 文件名
} DirEntry;

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