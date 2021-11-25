#include "shell.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFSIZE 64
#define PATH_SIZE 128
#define LSH_TOK_DELIM " \t\r\n\a"

#define MOUNT 0x01
#define UMOUNT 0x02

char* shellReadLine();
char** shellSplitLine(char* line);
int shellExecute(char** args);
int shellLaunch(char** args);

int shellNumFunc(Condition condition);
int shell_cd(char** args);
int shell_help(char** args);
int shell_man(char** args);
int shell_exit(char** args);
int shell_mkdisk(char** args);
int shell_format(char** args);
int shell_dump(char** args);
int shell_mount(char** args);
int shell_umount(char** args);

int getCurrentDirectory();
int getArgc(char** args);

/**************************** 函数变量声明 ********************************/

static Command commands[] = {
    {"cd", &shell_cd, ALL},
    {"help", &shell_help, ALL},
    {"exit", &shell_exit, ALL},
    {"man", &shell_man, ALL},
};
static int commands_size = 4;

int is_mounted = 0;

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

int shellNumFunc(Condition cond) {
  int count = 0;
  for (int i = 0; i < commands_size; i++) {
    if (commands[i].condition == cond) count++;
  }
  return count;
}

char* shellReadLine() {
  char* line = NULL;
  size_t bufsize = 0;  // 利用 getline 帮助我们分配缓冲区
  getline(&line, &bufsize, stdin);
  return line;
}

char** shellSplitLine(char* line) {
  int bufsize = BUFSIZE, position = 0;
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
      bufsize += BUFSIZE;
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

int shellExecute(char** args) {
  int i = 0;
  if (args[0] == NULL) {
    // 输入了空的命令
    return 1;
  }

  for (i = 0; i < shellNumFunc(ALL); i++) {
    // if (strcmp(args[0], shell_str[i]) == 0) {
    if (strcmp(args[0], commands[i].name) == 0) {
      return (*commands[i].func)(args);
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


  // 如果虚拟硬盘未挂载使用操作系统自带的命令
  if (!is_mounted) {
    return shellLaunch(args);
  }
  printf("Wrong command input. Type exit to exit this program\n");
  return 1;
}

int shellLaunch(char** args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("Shell");
  } else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int shell_help(char** args) {
  int i;
  printf("A simple shell for my ext2 filesystem\n");
  printf("Type program names and arguments, and hit enter\n");
  printf("The following are built in:\n");

  for (i = 0; i < shellNumFunc(ALL); i++) {
    printf("  %s\n", commands[i].name);
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

int shell_cd(char** args) {
  if (!is_mounted) {
    if (args[1] == NULL) {
      fprintf(stderr, "usage: cd <directory>\n");
    } else {
      if (chdir(args[1]) != 0) {
        perror("Wrong!!!");
      }
    }
  } else {
  }
  return 1;
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
  int argc = getArgc(args);

  if (argc != 2) {
    printf("usage: format <path-to-disk>\n");
    return 1;
  }
  diskLoad(g_disk, args[1]);

  int result = g_filesystem.format(&g_disk);

  if (result < 0) {
    print("%s formatting is failed\n", g_filesystem.name);
    return 1;
  }

  printf("successfully format the disk");
  return 1;
}

int shell_dump(char** args) {
  char* Args = {"hexdump", "ext2.dsk"};
  shellLaunch(Args);
  return 1;
}

int shell_mount(char** args) { in_disk = 1; }

int shell_umount(char** args) { in_disk = 0; }

/******************************* UTILS *************************************/

int getCurrentDirectory() {
  if (in_disk) {
    char pth[] = g_current_dir.name;
    memcpy(path_perfix, pth, sizeof(pth));
  } else {
    char* pth = getcwd(NULL, 0);
    memcpy(path_perfix, pth, sizeof(char) * PATH_SIZE);
  }
}

int getArgc(char** args) {
  int result = 0;
  while (args[result++] != NULL)
    ;
  return result;
}