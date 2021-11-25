#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/**************************** 函数声明 ******************************/

char* shellReadLine();
char** shellSplitLine(char* line);
int shellExecute(char** args);
int shellLaunch(char** args);

int shellNumFunc(Condition condition);
int shell_cd(char** args);
int shell_help(char** args);
int shell_man(char** args);
int shell_exit(char** args);

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
    printf("> ");
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
  }

  // 如果虚拟硬盘未挂载使用操作系统自带的命令
  if (!is_mounted) {
    return shellLaunch(args);
  }

  // 输入了错误的指令
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

  printf("Use the man command for information the programs.\n");
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

int shell_man(char** args) { return 1; }

int shell_exit(char** args) {
  printf("Bye\n");
  return 0;
}