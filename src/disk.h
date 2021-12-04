#ifndef __DISK_H__
#define __DISK_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

/**
 * @brief 记录磁盘文件信息
 *
 */
typedef struct Disk {
  char path[128];      // 磁盘路径

  // 读磁盘
  int (*write_disk)(struct Disk* disk, unsigned int block_idx, void* data);
  // 写磁盘
  int (*read_disk)(struct Disk* disk, unsigned int block_idx, void* data);
} Disk;

/**
 * @brief 将 data 中的数据写入 disk 的第 block_idx 个块
 *
 * @param disk 被写入数据的磁盘
 * @param sector_idx 写入的块位置
 * @param data 数据指针
 * @return int
 */
int writeDisk(Disk* disk, unsigned int block_idx, void* data);

/**
 * @brief 将 disk 的第 block_idx 个块的内容读到数据指针 data 中
 *
 * @param disk 被读出数据的磁盘
 * @param sector_idx 读取的块位置
 * @param data 数据指针
 * @return int
 */
int readDisk(Disk* disk, unsigned int block_idx, void* data);

/**
 * @brief 从路径初始化一个磁盘文件
 *
 * @param disk 磁盘指针
 * @param path 磁盘路径
 * @param number_of_sectors 扇区数量
 * @param sector_size 扇区大小
 * @param sectors_per_block 每个块有多少扇区
 * @return int
 */

/**
 * @brief 从路径初始化一个磁盘文件
 *
 * @param disk 磁盘指针
 * @param path 磁盘路径
 * @return int
 */
int makeDisk(Disk* disk, const char* path);

/**
 * @brief 从路径 path 加载一个 disk
 *
 * @param disk
 * @param path
 * @return int
 */
int loadDisk(Disk* disk, const char* path);

#endif  // __DISK_H__