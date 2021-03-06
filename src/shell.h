#ifndef __SHELL_H__
#define __SHELL_H__

// study from: https://brennan.io/2015/01/16/write-a-shell-in-c/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termio.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "ext2.h"

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"

typedef struct ShellEntry {
  Ext2Inode current_user;      // 当前位置
  Ext2FileSystem file_system;  // 文件系统
} ShellEntry;

typedef struct Command {
  char* name;
  int (*func)(char**);
} Command;

void exitDisplay();
int getCurrentPath(char* path);

int shell_mkdsk(char** args);
int shell_format(char** args);
int shell_mount(char** args);
int shell_umount(char** args);
int shell_ls(char** args);
int shell_tree(char** args);
int shell_mkdir(char** args);
int shell_help(char** args);
int shell_clear(char** args);
int shell_touch(char** args);
int shell_chmod(char** args);
int shell_info(char** args);
int shell_cd(char** args);
int shell_pwd(char** args);
int shell_rm(char** args);
int shell_rmdir(char** args);
int shell_write(char** args);
int shell_cat(char** args);
int shell_exit(char** args);

int shellFuncNum();
char* shellReadLine();
char** shellSplitLine(char* line);
int shellExecute(char** args);
int shellLaunch(char** args);
int shellLoop();
void shellStart();

#endif  // __SHELL_H__