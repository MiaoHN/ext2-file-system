#ifndef __EXT2_H__
#define __EXT2_H__

#include <stdio.h>
#include <termio.h>
#include <unistd.h>

#include "common.h"
#include "disk.h"

enum Ext2FileType {
  EXT2_FILE = 1,
  EXT2_DIR = 2,
};

/**
 * @brief 超级块占用一个 block，512 bytes
 *
 */
typedef struct Ext2SuperBlock {
  UINT32 inodes_count;       // 索引结点的总数
  UINT32 blocks_count;       // 文件系统块的总数
  UINT32 r_blocks_count;     // 为超级用户保留的块数
  UINT32 free_blocks_count;  // 空闲块总数
  UINT32 free_inodes_count;  // 空闲索引结点总数
  UINT32 first_data_block;   // 文件系统中第一个数据块
  UINT32 first_data_block_each_group;
  UINT32 log_block_size;    // 用于计算逻辑块的大小
  UINT32 log_frag_size;     // 用于计算片的大小
  UINT32 blocks_per_group;  // 每个组的块个数
  UINT32 frags_per_group;   // 每个组的片个数
  UINT32 inodes_per_group;  // 每个组的索引结点数
  UINT32 mtime;             // 文件系统的安装时间
  UINT32 wtime;             // 最后一次对超级块进行写的时间
  UINT16 mnt_count;         // 安装计数
  UINT16 max_mnt_count;     // 最大可安装计数
  UINT16 magic;   // 用于确定文件系统版本的标志 (ext2 -- 0xEF53)
  UINT16 state;   // 文件系统状态
  UINT16 errors;  // 当检测到错误时如何处理
  UINT16 minor_rev_level;  // 次版本号
  UINT32 last_check;       // 最后一次检测文件系统状态的时间
  UINT32 check_interval;  // 两次对文件系统状态进行检测的最大可能时间间隔
  UINT32 rev_level;  // 版本号，以此识别是否支持是否支持某些功能
  UINT16 def_fesuid;     // 保留块的默认用户标识 UID
  UINT16 def_fesgid;     // 保留块的默认用户组标识 GID
  UINT32 first_ino;      // 第一个非保留的索引结点号
  UINT16 inode_size;     // 索引结点结构的大小
  UINT16 block_group;    // 本 SuperBlock 所在的块组号
  UINT32 reserved[105];  // 保留
} Ext2SuperBlock;

/*
 * 组描述符，占用大小 32 bytes
 */
typedef struct Ext2GroupDesc {
  UINT32 block_bitmap;       // 指向该组中块位图所在块的指针
  UINT32 inode_bitmap;       // 指向该组中块结点位图所在块的指针
  UINT32 inode_table;        // 指向该组中结点的首块的指针
  UINT16 free_blocks_count;  // 本组空闲块的个数
  UINT16 free_inodes_count;  // 本组空闲索引结点的个数
  UINT16 used_dirs_count;    // 本组分配给目录的结点数
  UINT16 pad;                // 填充
  UINT32 reserved[3];        // 保留
} Ext2GroupDesc;

/*
 * gdt，占用一个 block，最多有16个组
 */
typedef struct Ext2GroupDescTable {
  Ext2GroupDesc table[16];
} Ext2GroupDescTable;

/**
 * @brief 索引节点，占用大小 128 bytes
 *
 */
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
  UINT16 reserved[24];          // 保留
} Ext2Inode;

typedef struct Ext2InodeTable {
  Ext2Inode table[NUMBER_OF_INODES];
} Ext2InodeTable;

/**
 * @brief 目录块，占用大小 32 bytes
 *
 */
typedef struct Ext2DirEntry {
  UINT32 inode;             // 索引结点号
  UINT16 rec_len;           // 目录项长度
  BYTE name_len;            // 文件名长度
  BYTE file_type;           // 文件类型(1: 普通文件 2: 目录 ...)
  char name[DIR_NAME_LEN];  // 文件名
  BYTE pad[9];              // 填充
} Ext2DirEntry;

/**
 * @brief 保存磁盘中的绝对位置
 *
 */
typedef struct Ext2Location {
  UINT32 block_idx;
  UINT32 offset;  // 单位 (byte)
} Ext2Location;

/**
 * @brief 文件系统
 *
 */
typedef struct Ext2FileSystem {
  Disk* disk;
} Ext2FileSystem;

// initialize -----------

int initSuperBlock(Ext2SuperBlock* super_block);
int initGdt(Ext2GroupDescTable* gdt, Ext2SuperBlock* super_block);
int initInodeBitmap(Disk* disk);
int initBlockBitmap(Disk* disk);

int initRootDir(Disk* disk);

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block);
int writeGdt(Disk* disk, Ext2GroupDescTable* gdt);

int getSuperBlock(Disk* disk, Ext2SuperBlock* super_block);
int getGdt(Disk* disk, Ext2GroupDescTable* gdt);

void getRootInode(Disk* disk, Ext2Inode* inode);

/**
 * @brief Get the Inode Index object
 *
 * @param disk
 * @param inode
 */
unsigned int getInodeIndex(Disk* disk, Ext2Inode* inode);

/**
 * @brief 在 disk 中添加一个 inode
 *
 * @param disk
 * @param inode 待添加的 inode 指针
 * @param location 添加的位置
 * @return int
 */
int writeInode(Disk*, Ext2Inode* inode, Ext2Location* location);

int getInode(Disk* disk, unsigned int index, Ext2Inode* inode);

/**
 * @brief 给 inode 添加一个目录块信息
 *
 * @param disk
 * @param inode
 * @param entry
 * @return unsigned int
 */
unsigned int addDirEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry);

/**
 * @brief 从磁盘中寻找空闲的 inode，并将其设为占用
 *
 * @param disk
 * @return Ext2Location 空闲 inode 的绝对位置信息
 */
Ext2Location getFreeInode(Disk* disk);

/**
 * @brief 从磁盘中寻找空闲块，并将其设为占用
 *
 * @param disk
 * @return Ext2Location 块的绝对位置信息
 */
Ext2Location getFreeBlock(Disk* disk);

int freeBlock(Disk* disk, int index);
int freeInode(Disk* disk, int index);

/**
 * @brief 从 block 数组中找到第 index 处的块
 *
 * @param disk
 * @param index
 * @return Ext2Location 目录块的绝对位置信息
 */
Ext2Location getDirEntryLocation(Disk* disk, unsigned int index,
                                 Ext2Inode* parent);

int getDirEntry(Disk* disk, unsigned int index, Ext2Inode* parent,
                Ext2DirEntry* entry);
int writeDirEntry(Disk* disk, unsigned int index, Ext2Inode* parent,
                  Ext2DirEntry* entry);

int getCurrentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry);
int getParentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry);
int writeCurrentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry);
int writeParentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry);

int writeFile(Disk* disk, Ext2Inode* inode);
int readFile(Disk* disk, Ext2Inode* inode);

// shell 调用的操作

int ext2Format(Disk* disk);
int ext2Ls(Ext2FileSystem* file_system, Ext2Inode* current);
int ext2Mount(Ext2FileSystem* file_system, Ext2Inode* current, char* path);
int ext2Mkdir(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Touch(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Rmdir(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Rm(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int deleteDirEntry(Ext2FileSystem* file_system, Ext2Inode* current, char* name,
                   int type);
int ext2Open(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Write(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Cat(Ext2FileSystem* file_system, Ext2Inode* current, char* name);

// Bitmap 操作

void getInodeBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]);
void getBlockBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]);
void setBlockBitmap(Disk* disk, unsigned int index, int value);
void setInodeBitmap(Disk* disk, unsigned int index, int value);
void writeInodeBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]);
void writeBlockBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]);

// 位操作

// 将位图 block 位于 index 处的值设为 value
int setBit(BYTE bitmap[BLOCK_SIZE], int index, int value);
// 得到从左往右第一个 0 的位置
int getOffset(BYTE byte);

int writeBlock(Disk* disk, unsigned int block_idx, void* block);
int readBlock(Disk* disk, unsigned int block_idx, void* block);
char getCh();
#endif  // __EXT2_H__