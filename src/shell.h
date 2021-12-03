#ifndef __SHELL_H__
#define __SHELL_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termio.h>
#include <time.h>
#include <unistd.h>

#include "ext2.h"

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

typedef struct ShellEntry {
  Ext2Inode current_user;      // 当前位置
  Ext2FileSystem file_system;  // 文件系统
} ShellEntry;

/***************** 全局变量 *****************/

static ShellEntry shell_entry;
int is_mounted = 0;

void exitDisplay() {
  printf("Thank you for using!\n");
  return;
}

int shell_makedisk(char **args) {
  if (args[1] == NULL) {
    printf("usage: makedisk <disk-name>\n");
    return 1;
  }

  Disk disk;
  makeDisk(&disk, args[1], NUMBER_OF_BLOCKS);
  printf("Successfully make a disk named %s !\n", args[1]);
  return 1;
}

int shell_format(char **args) {
  if (args[1] == NULL) {
    printf("usage: format <disk-name>\n");
    return 1;
  }
  Disk disk;
  loadDisk(&disk, args[1]);
  format(&disk);

  printf("Successfully format disk %s to Ext2\n", args[1]);
  return 1;
}

int shell_mount(char **args) {
  if (args[1] == NULL) {
    printf("usage: mount <disk-name>\n");
    return 1;
  }
  if (is_mounted == 1) {
    printf("You are already mounted in the Ext2 File System\n");
    return 1;
  }

  ext2Mount(&shell_entry.file_system, &shell_entry.current_user, args[1]);

  is_mounted = 1;
  printf("Successfully mount the disk \"%s\"\n", args[1]);
  return 1;
}

int shell_ls(char **args) {
  if (is_mounted == 0) {
    printf("The file system isn't mounted!\n");
    return 1;
  }
  ext2Ls(&shell_entry.file_system, &shell_entry.current_user);
  return 1;
}

int shell_mkdir(char **args) {
  if (is_mounted == 0) {
    printf("The file system isn't mounted!\n");
    return 1;
  }

  if (args[1] == NULL) {
    printf("usage: mkdir <dir-name>\n");
    return 1;
  }

  ext2Mkdir(&shell_entry.file_system, &shell_entry.current_user, args[1]);
  printf("Successfully make directory named \"%s\"\n", args[1]);
  return 1;
}

typedef struct Command {
  char *name;
  int (*func)(char **);
} Command;

static Command commands[] = {
    {"ls", &shell_ls},         {"makedisk", &shell_makedisk},
    {"format", &shell_format}, {"mount", &shell_mount},
    {"mkdir", &shell_mkdir},
};

int shellFuncNum() { return sizeof(commands) / sizeof(Command); }

char *shellReadLine() {
  char *line = NULL;
  size_t bufsize = 0;  // 利用 getline 帮助我们分配缓冲区
  getline(&line, &bufsize, stdin);
  return line;
}

char **shellSplitLine(char *line) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char *));
  char *token;

  if (!tokens) {
    fprintf(stderr, "lsh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char *));
      if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int shellExecute(char **args) {
  int i = 0;
  if (args[0] == NULL) {
    // 输入了空的命令
    return 1;
  }

  // 如果虚拟硬盘未挂载使用操作系统自带的命令
  if (!is_mounted) {
    for (i = 0; i < shellFuncNum(); i++) {
      if (strcmp(args[0], commands[i].name) == 0) {
        return (*commands[i].func)(args);
      }
    }
    // return shellLaunch(args);
    return 1;
  } else {
    for (i = 0; i < shellFuncNum(); i++) {
      if (strcmp(args[0], commands[i].name) == 0) {
        return (*commands[i].func)(args);
      }
    }
  }
  return 1;
}

int shellLoop() {
  char *line;
  char **args;
  int status;

  do {
    char current_path[32];
    // getCurrentPath(current_path);
    printf("%s > ", current_path);
    line = shellReadLine();
    args = shellSplitLine(line);
    status = shellExecute(args);

    free(line);
    free(args);
  } while (status);
  return 0;
}

void shellStart() {
  printf("Hello! Welcome to Ext2 like file system!\n");
  shellLoop();
  exitDisplay();
}

#endif  // __SHELL_H__