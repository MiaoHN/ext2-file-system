/**
 * @file disk.h
 * @author MiaoHN (582418227@qq.com)
 * @brief 模拟一个磁盘的相关操作，包括磁盘的读写
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __DISK_H__
#define __DISK_H__

#include "type.h"

typedef struct {
  DATA data;             // 指向数据的指针
  SECTOR num_of_sector;  // 磁盘中扇区的数量
  int bytes_per_sector;  // 每个扇区的比特数

  int (*write_sector)(struct disk*, SECTOR, DATA);
  int (*read_sector)(struct disk*, SECTOR, DATA);
} disk;

/**
 * @brief 初始化磁盘
 *
 * @return int
 */
int disk_init();

/**
 * @brief 从扇区写内容
 *
 * @param _disk 被写入磁盘
 * @param sector 被写入扇区
 * @param data 要写入的状态
 * @return int 写入失败返回 1
 */
int disk_write(disk* _disk, SECTOR sector, DATA data);

/**
 * @brief 从扇区读内容
 *
 * @param _disk 被读磁盘
 * @param sector 被读扇区
 * @param data 被读出的数据
 * @return int 读取失败返回 1
 */
int disk_read(disk* _disk, SECTOR sector, DATA data);

#endif  // __DISK_H__