#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"

/**************************** 函数声明 ******************************/

/**
 * @brief 从控制台读入一行内容
 *
 * @return char* 读入的字符串
 */
char* shell_read_line();

/**
 * @brief 将读入的字符串分组
 *
 * @param line 读入的字符串
 * @return char**
 */
char** shell_split_line(char* line);

/**
 * @brief 执行控制台输入的指令
 *
 * @param args
 * @return int 运行状态，如果退出则返回 0
 */
int shell_execute(char** args);

/**
 * @brief 统计可用函数的数量
 *
 * @return int
 */
int shell_num_func();

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

/**************************** 函数变量声明 ********************************/

char* shell_str[] = {"cd", "help", "exit", "man"};
int (*shell_func[])(char**) = {&shell_cd, &shell_help, &shell_exit, &shell_man};

/******************************* 函数实现 *********************************/

int shell_loop() {
  char* line;
  char** args;
  int status;

  shell_help(NULL);

  do {
    printf("> ");
    line = shell_read_line();
    args = shell_split_line(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
  return 0;
}

int shell_num_func() { return sizeof(shell_str) / sizeof(char*); }

char* shell_read_line() {
  char* line = NULL;
  size_t bufsize = 0;  // 利用 getline 帮助我们分配缓冲区
  getline(&line, &bufsize, stdin);
  return line;
}

char** shell_split_line(char* line) {
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

int shell_execute(char** args) {
  int i = 0;
  if (args[0] == NULL) {
    // an empty command was entered
    return 1;
  }

  for (i = 0; i < shell_num_func(); i++) {
    if (strcmp(args[0], shell_str[i]) == 0) {
      return (*shell_func[i])(args);
    }
  }

  // wrong command.
  printf("Wrong command input. Type exit to exit this program\n");
  return 1;
}

int shell_help(char** args) {
  int i;
  printf("A simple shell for my ext2 filesystem\n");
  printf("Type program names and arguments, and hit enter\n");
  printf("The following are built in:\n");

  for (i = 0; i < shell_num_func(); i++) {
    printf("  %s\n", shell_str[i]);
  }

  printf("Use the man command for information the programs.\n");
  return 1;
}

int shell_cd(char** args) { return 1; }

int shell_man(char** args) { return 1; }

int shell_exit(char** args) {
  printf("Bye\n");
  return 0;
}