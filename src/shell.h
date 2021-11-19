#ifndef __SHELL_H__
#define __SHELL_H__

int shell_cd(int argc, char** argv);
int shell_mkdir(int argc, char** argv);

typedef struct {
  char* str;
  int (*func)(int argc, char** argv);
} command;

// shell 中可使用的命令和解释函数
command global_commands[] = {
    {"cd", shell_cd},
    {"mkdir", shell_mkdir},
};

void start_shell() {
  char buf[256];
  while (1) {
  fgets(buf, 256, stdin);
  }
}

#endif  // __SHELL_H__