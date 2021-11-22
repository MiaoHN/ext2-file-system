/**
 * @file debug.h
 * @author MiaoHN (582418227@qq.com)
 * @brief
 * @version 0.1
 * @date 2021-11-22
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"
#include "disk.h"

int dumpDisk(Disk* disk, int offset, int bytes);

int printDump(BYTE* data, int offset, int bytes);

#endif  // __DEBUT_H__
