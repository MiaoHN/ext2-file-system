/**
 * @file common.h
 * @author MiaoHN (582418227@qq.com)
 * @brief 定义公共的常量
 * @version 0.1
 * @date 2021-11-20
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#define DISK_SIZE (SECTOR_SIZE * NUMBER_OF_SECTORS)

#define BYTES_PER_SECTOR 512
#define SECTORS_PER_BLOCK 1

#define SECTOR_SIZE BYTES_PER_SECTOR
#define BLOCK_SIZE (SECTORS_PER_BLOCK * BYTES_PER_SECTOR)

#define NUMBER_OF_SECTORS 4096
#define NUMBER_OF_BLOCKS (NUMBER_OF_SECTORS * SECTORS_PER_BLOCK)


#endif  // __COMMON_H__