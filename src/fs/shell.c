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

int shellNumFunc();
int shell_cd(char** args);
int shell_help(char** args);
int shell_man(char** args);
int shell_exit(char** args);

/**************************** 函数变量声明 ********************************/

char* shell_str[] = {"cd", "help", "exit", "man"};
int (*shell_func[])(char**) = {&shell_cd, &shell_help, &shell_exit, &shell_man};

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

int shellNumFunc() { return sizeof(shell_str) / sizeof(char*); }

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
    // an empty command was entered
    return 1;
  }

  for (i = 0; i < shellNumFunc(); i++) {
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

  for (i = 0; i < shellNumFunc(); i++) {
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