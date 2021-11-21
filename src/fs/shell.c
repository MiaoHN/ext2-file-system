#include "shell.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

int in_disk = 0;

#define PATH_SIZE 128

char path_perfix[PATH_SIZE] = "";

/********************************* 函数声明 ***********************************/

int getCurrentDirectory() {
  if (in_disk) {
    char pth[] = "IN DISK";
    memcpy(path_perfix, pth, sizeof(pth));
  } else {
    char* pth = getcwd(NULL, 0);
    memcpy(path_perfix, pth, sizeof(char) * PATH_SIZE);
  }
}

/**
 * @brief 从控制台读入一行内容
 *
 * @return char* 读入的字符串
 */
char* shellReadLine();

/**
 * @brief 将读入的字符串分组
 *
 * @param line 读入的字符串
 * @return char**
 */
char** shellSplitLine(char* line);

/**
 * @brief 若工作目录位于虚拟磁盘中使用该函数执行命令
 *
 * @param args
 * @return int
 */
int diskExecute(char** args);

/**
 * @brief 若工作目录不在虚拟磁盘中使用该函数执行命令
 *
 * @param args
 * @return int
 */
int outExecute(char** args);

/**
 * @brief 执行控制台输入的指令
 *
 * @param args
 * @return int 运行状态，如果退出则返回 0
 */
int shellExecute(char** args);

/**
 * @brief 使用 execvp 执行操作系统自带的程序
 *
 * @param args
 * @return int
 */
int shellLaunch(char** args);

/**
 * @brief 统计在虚拟磁盘中可用函数的数量
 *
 * @return int
 */
int diskNumFunc();

/**
 * @brief 统计可用函数的数量
 *
 * @return int
 */
int outNumFunc();

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

int manBuiltin(char** args);

/**
 * @brief 退出程序
 *
 * @param args 参数列表
 * @return int 0
 */
int shell_exit(char** args);

/**
 * @brief 创建一个硬盘，默认按照参数路径新建一个磁盘文件，
 *
 * TODO: 硬盘已经存在则直接读取之
 *
 * @param args
 * @return int
 */
int shell_mkdisk(char** args);

/**
 * @brief 格式化模拟硬盘
 *
 * TODO: 添加硬盘路径参数
 *
 * @param args
 * @return int
 */
int shell_format(char** args);

/**
 * @brief 使用操作系统提供的 hexdump
 *
 * @param args
 * @return int
 */
int shell_dump(char** args);

/**
 * @brief 挂载并进入虚拟硬盘
 *
 * @param args
 * @return int
 */
int shell_mount(char** args);

/**
 * @brief 取消挂载虚拟硬盘
 *
 * @param args
 * @return int
 */
int shell_umount(char** args);

/**************************** 函数变量声明 ********************************/

// 在 ext2 磁盘中的可用命令
char* shell_disk_str[] = {"cd", "help", "exit", "man", "dump", "umount"};
int (*shell_disk_func[])(char**) = {&shell_cd,  &shell_help, &shell_exit,
                                    &shell_man, &shell_dump, &shell_umount};

// 在 ext2 磁盘之外的重定义命令
char* shell_out_str[] = {"cd",     "help",   "exit", "man",
                         "mkdisk", "format", "mount"};
int (*shell_out_func[])(char**) = {&shell_cd,   &shell_help,   &shell_exit,
                                   &shell_man,  &shell_mkdisk, &shell_format,
                                   &shell_mount};

static FileSystem g_file_system;
static Disk disk;

/******************************* 函数实现 *********************************/

int shellLoop() {
  char* line;
  char** args;
  int status;

  shell_help(NULL);

  do {
    getCurrentDirectory();
    printf("%s > ", path_perfix);
    line = shellReadLine();
    args = shellSplitLine(line);
    status = shellExecute(args);

    free(line);
    free(args);
  } while (status);
  return 0;
}

int diskNumFunc() { return sizeof(shell_disk_str) / sizeof(char*); }
int outNumFunc() { return sizeof(shell_out_str) / sizeof(char*); }

char* shellReadLine() {
  char* line = NULL;
  size_t bufsize = 0;  // 利用 getline 帮助我们分配缓冲区
  getline(&line, &bufsize, stdin);
  return line;
}

char** shellSplitLine(char* line) {
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char** tokens = malloc(bufsize * sizeof(char*));
  char* token;

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
      tokens = realloc(tokens, bufsize * sizeof(char*));
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

int shellLaunch(char** args) {
  int status;
  pid_t pid = fork();

  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      printf("Wrong command input. Type exit to exit this program\n");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("wrong");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

int diskExecute(char** args) {
  for (int i = 0; i < diskNumFunc(); i++) {
    if (strcmp(args[0], shell_disk_str[i]) == 0) {
      return (*shell_disk_func[i])(args);
    }
  }
  // wrong command.
  printf("Wrong command input. Type exit to exit this program\n");
  return 1;
}

int outExecute(char** args) {
  for (int i = 0; i < outNumFunc(); i++) {
    if (strcmp(args[0], shell_out_str[i]) == 0) {
      return (*shell_out_func[i])(args);
    }
  }
  return shellLaunch(args);
}

int shellExecute(char** args) {
  if (args[0] == NULL) {
    // an empty command was entered
    return 1;
  }

  if (in_disk) {
    // 如果已经挂载到虚拟磁盘
    return diskExecute(args);
  } else {
    return outExecute(args);
  }
}

int shell_help(char** args) {
  int i;
  printf("A simple shell for my ext2 filesystem\n");
  if (in_disk) {
    printf("Now you are in the virtual disk.\n");
    printf("The following are available:\n");
    for (i = 0; i < diskNumFunc(); i++) {
      printf("  %s\n", shell_disk_str[i]);
    }
    printf("Use help command to see this information again.\n");
    printf("Use the man command for information the programs.\n");
  } else {
    printf("Now you are NOT in the virtual disk.\n");
    printf("You can use commands OS supported.\n");
    printf("The following are built in or modified:\n");
    for (i = 0; i < outNumFunc(); i++) {
      printf("  %s\n", shell_out_str[i]);
    }
    printf("Use help command to see this information again.\n");
    printf("Use the man command for information the programs.\n");
  }
  return 1;
}

int shell_cd(char** args) {
  if (args[1] == NULL) {
    fprintf(stderr, "expected argument to \"cd\"\n");
  } else if (in_disk) {
  } else {
    if (chdir(args[1]) != 0) {
      perror("chdir error");
    }
  }
  return 1;
}

int shell_man(char** args) {
  if (args[1] == NULL) {
    printf("usage: man <command>\n");
    return 1;
  }
  if (in_disk) {
    return manBuiltin(args);
  } else {
    for (int i = 0; i < outNumFunc(); i++) {
      if (strcmp(args[1], shell_out_str[i]) == 0) {
        return manBuiltin(args);
      }
    }
    return shellLaunch(args);
  }
}

int manBuiltin(char** args) {
  for (int i = 0; i < outNumFunc(); i++) {
    if (strcmp(args[1], shell_out_str[i]) == 0) {
      // TODO
    }
  }
}

int shell_exit(char** args) {
  printf("Bye\n");
  return 0;
}

int shell_mkdisk(char** args) {
  diskInit(&disk, 4096, 512, "./ext2.dsk");
  return 1;
}

int shell_format(char** args) {
  fileSystemFormat(&disk);
  return 1;
}

int shell_dump(char** args) {
  char* Args = {"hexdump", "ext2.dsk"};
  shellLaunch(Args);
  return 1;
}

int shell_mount(char** args) { in_disk = 1; }

int shell_umount(char** args) { in_disk = 0; }