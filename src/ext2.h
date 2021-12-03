#ifndef __EXT2_H__
#define __EXT2_H__

#include "common.h"
#include "disk.h"

enum Ext2FileType {
  EXT2_FILE = 1,
  EXT2_DIR = 2,
};

/**
 * @brief 超级块占用一个 block
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

/**
 * @brief 组描述符，占用大小 32 字节
 *
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

/**
 * @brief gdt，占用一个 block，最多有16个组
 *
 */
typedef struct Ext2GroupDescTable {
  Ext2GroupDesc table[16];
} Ext2GroupDescTable;

/**
 * @brief 索引节点，占用大小 128 字节
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
  UINT16 reserved[9];           // 保留
} Ext2Inode;

typedef struct Ext2InodeTable {
  Ext2Inode table[NUMBER_OF_INODES];
} Ext2InodeTable;

/**
 * @brief 目录块，占用大小 32 字节
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
  Ext2SuperBlock* super_block;
  Ext2GroupDescTable* gdt;
} Ext2FileSystem;

// initialize -----------

int initSuperBlock(Ext2SuperBlock* super_block);
int initGdt(Ext2GroupDescTable* gdt, Ext2SuperBlock* super_block);
int initInodeBitmap(Disk* disk, Ext2SuperBlock* super_block);
int initBlockBitmap(Disk* disk, Ext2SuperBlock* super_block);

int initRootDir(Disk* disk, Ext2SuperBlock* super_block);

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block);

int writeGdt(Disk* disk, Ext2SuperBlock* super_block, Ext2GroupDescTable* gdt);

int getRootInode(Ext2FileSystem* file_system, Ext2Inode* inode);

int addInode(Ext2FileSystem* file_system, Ext2Inode* inode,
             Ext2Location* location);

unsigned int addDirEntry(Ext2FileSystem* file_system, Ext2Inode* inode,
                         unsigned int inode_idx, Ext2Location inode_location,
                         int type, char* name);

Ext2Location findFreeInode(Ext2FileSystem* file_system, unsigned int* idx);

Ext2Location findFreeBlock(Ext2FileSystem* file_system, unsigned int* idx);

Ext2Location findDirEntry(Ext2FileSystem* file_system, unsigned int index,
                          unsigned int block[8]);

/**
 * @brief 将 disk 初始化文件系统
 *
 * @param disk
 * @return int
 */
int format(Disk* disk);

int ext2Ls(Ext2FileSystem* file_system, Ext2Inode* current);

int ext2Mount(Ext2FileSystem* file_system, Ext2Inode* current, char* path);

int ext2Mkdir(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Touch(Ext2FileSystem* file_system, Ext2Inode* current, char* name);

int ext2Open(Ext2FileSystem* file_system, Ext2Inode* current, char* name);
int ext2Close(Ext2FileSystem* file_system, Ext2Inode* currnet);

int setBit(BYTE* block, int index, int value);
int getOffset(BYTE byte);

int writeBlock(Disk* disk, unsigned int block_idx, void* block);
int readBlock(Disk* disk, unsigned int block_idx, void* block);

#endif  // __EXT2_H__