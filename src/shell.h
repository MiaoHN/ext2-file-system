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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * @brief shell 开始运行
 *
 * @return int
 */
int shell_loop();

/************************ funcs builtin ***************************/

/**
 * @brief
 *
 * @param args
 * @return int 1
 */
int shell_cd(char** args);

/**
 * @brief
 *
 * @param args
 * @return int 1
 */
int shell_help(char** args);

/**
 * @brief 简单查询函数的用法
 *
 * @param args 参数列表
 * @return int 1
 */
int shell_man(char** args);

/**
 * @brief 退出程序
 *
 * @param args 参数列表
 * @return int 0
 */
int shell_exit(char** args);


#endif  // __SHELL_H__