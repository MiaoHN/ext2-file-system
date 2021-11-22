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

#include <sys/wait.h>
#include <unistd.h>

#include "disk.h"
#include "ext2.h"

typedef struct ShellFiletime {
  unsigned short year;
  unsigned char month;
  unsigned char day;

  unsigned char hour;
  unsigned char minute;
  unsigned char second;
} ShellFiletime;

typedef struct ShellEntry {
  struct ShellEntry* parent;

  unsigned char name[256];
  unsigned char isDirectory;
  unsigned int size;

  unsigned short permition;
  ShellFiletime create_time;
  ShellFiletime modify_time;
  char data[1024];
} ShellEntry;

typedef struct ShellEntryListItem {
  ShellEntry entry;
  struct ShellEntryListItem* next;
} ShellEntryListItem;

typedef struct ShellEntryList {
  unsigned int count;
  ShellEntryListItem* first;
  ShellEntryListItem* last;
} ShellEntryList;

struct ShellFileOperations;

typedef struct ShellFileSystemOperations {
  int (*read_dir)(Disk*, struct ShellFileSystemOperations*, const ShellEntry*,
                  ShellEntryList*);
  int (*stat)(Disk*, struct ShellFileSystemOperations*, unsigned int*,
              unsigned int*);
  int (*mkdir)(Disk*, struct ShellFileSystemOperations*, const ShellEntry*,
               const char*, ShellEntry*);
  int (*rmdir)(Disk*, struct ShellFileSystemOperations*, const ShellEntry*,
               const char*);
  int (*lookup)(Disk*, struct ShellFileSystemOperations*, const ShellEntry*,
                ShellEntry*, const char*);
  struct ShellFileOperations* file_operations;
  DATA data;
} ShellFileSystemOperations;

typedef struct ShellFileOperations {
  int (*create)(Disk*, ShellFileSystemOperations*, const ShellEntry*,
                const char*, ShellEntry*);
  int (*remove)(Disk*, ShellFileSystemOperations*, const ShellEntry*,
                const char*);
  int (*read)(Disk*, ShellFileSystemOperations*, const ShellEntry*, ShellEntry*,
              unsigned long, unsigned long, char*);
  int (*write)(Disk*, ShellFileSystemOperations*, const ShellEntry*,
               ShellEntry*, unsigned long, unsigned long, const char*);
} ShellFileOperations;

typedef struct ShellFileSystem {
  char* name;
  int (*mount)(Disk*, ShellFileSystemOperations*, ShellEntry*);
  void (*umount)(Disk*, ShellFileSystemOperations*);
  int (*format)(Disk*);
} ShellFileSystem;

int initEntryList(ShellEntryList* list);
int addEntryList(ShellEntryList*, struct ShellEntry*);
void releaseEntryList(ShellEntryList*);

/**
 * @brief shell 开始运行
 *
 * @return int
 */
int shellLoop();

#endif  // __SHELL_H__