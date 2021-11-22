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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "type.h"

typedef struct Disk {
  char* path;  // 磁盘路径文件

  int number_of_sectors;     // 磁盘中扇区的数量
  int bytes_per_sector;  // 每个扇区的比特数

  int (*write_sector)(struct Disk*, int, BYTE*);  // 读取操作
  int (*read_sector)(struct Disk*, int, BYTE*);   // 写操作
} Disk;

int diskInit(Disk* disk, int num_of_sectors, int byte_per_sector, char* path);

int diskRemove(Disk* disk);

int diskWrite(Disk* disk, int sector, BYTE* data);

int diskRead(Disk* disk, int sector, BYTE* data);

#endif  // __DISK_H__