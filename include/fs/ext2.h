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

#include "common.h"
#include "disk.h"
#include "type.h"

#define EXT2_N_BLOCKS 2323

#define NAME_LEN 255

#define EXT2_VALID_FS 0x0001
#define EXT2_ERROR_FS 0x0002

/**
 * @brief 超级块
 *
 */
typedef struct Ext2SuperBlock {
  UINT32 inodes_count;       // 索引结点的总数
  UINT32 blocks_count;       // 文件系统块的总数
  UINT32 r_blocks_count;     // 为超级用户保留的块数
  UINT32 free_blocks_count;  // 空闲块总数
  UINT32 free_inodes_count;  // 空闲索引结点总数
  UINT32 first_data_block;   // 文件系统中第一个数据块
  UINT32 log_block_size;     // 用于计算逻辑块的大小
  UINT32 log_frag_size;      // 用于计算片的大小
  UINT32 blocks_per_group;   // 每个组的块个数
  UINT32 frags_per_group;    // 每个组的片个数
  UINT32 inodes_per_group;   // 每个组的索引结点数
  UINT32 mtime;              // 文件系统的安装时间
  UINT32 wtime;              // 最后一次对超级块进行写的时间
  UINT16 mnt_count;          // 安装计数
  UINT16 max_mnt_count;      // 最大可安装计数
  UINT16 magic;   // 用于确定文件系统版本的标志 (ext2 -- 0xEF53)
  UINT16 state;   // 文件系统状态
  UINT16 errors;  // 当检测到错误时如何处理
  UINT16 minor_rev_level;  // 次版本号
  UINT32 last_check;       // 最后一次检测文件系统状态的时间
  UINT32 check_interval;  // 两次对文件系统状态进行检测的最大可能时间间隔
  UINT32 rev_level;  // 版本号，以此识别是否支持是否支持某些功能
  UINT16 def_fesuid;      // 保留块的默认用户标识 UID
  UINT16 def_fesgid;      // 保留块的默认用户组标识 GID
  UINT32 first_ino;       // 第一个非保留的索引结点号
  UINT16 inode_size;      // 索引结点结构的大小
  UINT16 block_group_nr;  // 本 SuperBlock 所在的块组号
  UINT32 reserved[230];   // 保留
} Ext2SuperBlock;

/**
 * @brief 组描述符
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
  UINT32 reserved;           // 保留
} Ext2GroupDesc;

/**
 * @brief 索引结点
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
  UINT32 reserved;              // 保留
} Ext2Inode;

/**
 * @brief 目录块
 *
 */
typedef struct Ext2DirEntry {
  UINT32 inode;         // 索引结点号
  UINT16 rec_len;       // 目录项长度
  BYTE name_len;        // 文件名长度
  BYTE file_type;       // 文件类型(1: 普通文件 2: 目录 ...)
  char name[NAME_LEN];  // 文件名
} Ext2DirEntry;

#endif  // __EXT2_H__