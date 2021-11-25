/**
 * @file shell.h
 * @author MiaoHN (582418227@qq.com)
 * @brief 一个简单的内置 shell，用于操作 ext2 文件系统
 * @version 0.1
 * @date 2021-11-19
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef __SHELL_H__
#define __SHELL_H__

typedef enum Condition {
  MOUNTED = 0,
  UMOUNTED = 1,
  ALL = 2,
} Condition;

typedef struct Command {
  char* name;           // 函数名称
  int (*func)(char**);  // 函数指针
  Condition condition;  // 函数的使用情况：是否新文件系统已经挂载
} Command;

/**
 * @brief shell 开始运行
 *
 * @return int
 */
int shellLoop();

#endif  // __SHELL_H__