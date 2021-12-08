#include "shell.h"

static Command commands[] = {
    {"ls", &shell_ls},       {"mkdsk", &shell_mkdsk}, {"format", &shell_format},
    {"mount", &shell_mount}, {"mkdir", &shell_mkdir}, {"touch", &shell_touch},
    {"cd", &shell_cd},       {"exit", &shell_exit},   {"umount", &shell_umount},
    {"rmdir", &shell_rmdir}, {"rm", &shell_rm},       {"write", &shell_write},
    {"cat", &shell_cat},     {"pwd", &shell_pwd},     {"help", &shell_help},
    {"clear", &shell_clear}, {"chmod", &shell_chmod}, {"info", &shell_info},
    {"tree", &shell_tree},
};

char* path_stack[256];
static int stack_top = -1;
static ShellEntry shell_entry;
int is_mounted = 0;

void exitDisplay() {
  printf("Thank you for using!\n");
  return;
}

int getCurrentPath(char* path) {
  if (!is_mounted) {
    strcpy(path, "UNMOUNTED");
  } else {
    strcpy(path, path_stack[stack_top]);
  }
  return 1;
}

int shell_mkdsk(char** args) {
  if (is_mounted == 1) {
    printf("You're already mount on the disk, please umount before mkdsk\n");
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: makedisk <disk-name>\n");
    return 1;
  }

  Disk disk;
  makeDisk(&disk, args[1]);
  return 1;
}

int shell_format(char** args) {
  if (is_mounted == 1) {
    printf("You're already mount on the disk, please umount before format\n");
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: format <disk-name>\n");
    return 1;
  }
  if (access(args[1], F_OK) == -1) {
    printf(
        "The disk named \"%s\" isn't exist. Please use \"mkdsk\" to create a "
        "disk first\n",
        args[1]);
    return 1;
  }
  Disk disk;
  loadDisk(&disk, args[1]);
  ext2Format(&disk);

  return 1;
}

int shell_mount(char** args) {
  if (is_mounted == 1) {
    printf("You are already mounted in the Ext2 File System\n");
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: mount <disk-name>\n");
    return 1;
  }
  if (access(args[1], F_OK) == -1) {
    printf(
        "The disk named \"%s\" isn't exist. Please use \"mkdsk\" to create a "
        "disk first\n",
        args[1]);
    return 1;
  }

  // 检查是否已经 format 过
  if (checkExt2(args[1]) == FAILURE) {
    printf("Please format the disk first!\n");
    return 1;
  }
  ext2Mount(&shell_entry.file_system, &shell_entry.current_user, args[1]);

  is_mounted = 1;
  shell_help(NULL);
  return 1;
}

int shell_umount(char** args) {
  if (is_mounted == 0) {
    printf("The Ext2 File System is not mounted\n");
    return 1;
  }
  is_mounted = 0;
  stack_top = 0;
  path_stack[stack_top] = "/";
  printf("Successfully umount from the virtual file system\n");
  return 1;
}

int shell_ls(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }
  ext2Ls(&shell_entry.file_system, &shell_entry.current_user);
  return 1;
}

int shell_tree(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }
  ext2Tree(&shell_entry.file_system, 0, 0, &shell_entry.current_user);
  return 1;
}

int shell_mkdir(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }

  if (args[1] == NULL) {
    printf("usage: mkdir <dir-name>\n");
    return 1;
  }

  ext2Mkdir(&shell_entry.file_system, &shell_entry.current_user, args[1]);
  return 1;
}

int shell_touch(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }

  if (args[1] == NULL) {
    printf("usage: touch <file-name>\n");
    return 1;
  }

  ext2Touch(&shell_entry.file_system, &shell_entry.current_user, args[1]);
  return 1;
}

int shell_chmod(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }

  if (args[2] == NULL) {
    printf("usage: chmod <mode> <file-name>\n");
    printf(" <mode>:\n");
    printf("       0 : Toggle Readable\n");
    printf("       1 : Toggle Writable\n");
    return 1;
  }
  int mode;
  if (!strcmp(args[1], "0")) {
    mode = 0;
  } else if (!strcmp(args[1], "1")) {
    mode = 1;
  } else {
    printf("usage: chmod <mode> <file-name>\n");
    printf(" <mode>:\n");
    printf("       0 : Toggle Readable\n");
    printf("       1 : Toggle Writable\n");
    return 1;
  }

  ext2Chmod(&shell_entry.file_system, &shell_entry.current_user, mode, args[1]);
  return 1;
}

int shell_info(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }

  printDiskInfo(shell_entry.file_system.disk);
  return 1;
}

int shell_cd(char** args) {
  if (is_mounted == 0) {
    if (args[1] == NULL) {
      fprintf(stderr, "Shell: expected argument to \"cd\"\n");
    } else {
      if (chdir(args[1]) != 0) {
        perror("shell");
      }
    }
    return 1;
  }

  if (args[1] == NULL) {
    printf("usage: cd <dir-name>\n");
    return 1;
  }
  if (!strcmp(args[1], "..")) {
    // 返回上级目录，栈顶减小
    if (stack_top > 0)
      stack_top--;
    ext2Open(&shell_entry.file_system, &shell_entry.current_user, args[1]);
    return 1;
  } else if (!strcmp(args[1], ".")) {
    // 当前目录，不处理
    return 1;
  } else if (!strcmp(args[1], "/")) {
    stack_top = 0;
    path_stack[stack_top] = "/";
    ext2Open(&shell_entry.file_system, &shell_entry.current_user, args[1]);
  }

  if (ext2Open(&shell_entry.file_system, &shell_entry.current_user, args[1]) ==
      SUCCESS) {
    stack_top++;
    path_stack[stack_top] = malloc(128 * sizeof(char*));
    strcpy(path_stack[stack_top], args[1]);
  }
  return 1;
}

int shell_rm(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: rm <file-name>\n");
    return 1;
  }

  ext2Rm(&shell_entry.file_system, &shell_entry.current_user, args[1]);

  return 1;
}

int shell_rmdir(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: rm <file-name>\n");
    return 1;
  }

  ext2Rmdir(&shell_entry.file_system, &shell_entry.current_user, args[1]);

  return 1;
}

int shell_write(char** args) {
  if (is_mounted == 0) {
    printf("The file system isn't mounted!\n");
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: write <file-name>\n");
    return 1;
  }

  ext2Write(&shell_entry.file_system, &shell_entry.current_user, args[1]);

  return 1;
}

int shell_cat(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }
  if (args[1] == NULL) {
    printf("usage: cat <file-name>\n");
    return 1;
  }

  ext2Cat(&shell_entry.file_system, &shell_entry.current_user, args[1]);

  return 1;
}

int shell_pwd(char** args) {
  if (is_mounted == 0) {
    shellLaunch(args);
    return 1;
  }

  for (int i = 0; i < stack_top; i++) {
    printf("%s/", path_stack[i]);
  }
  printf("\n");

  return 1;
}

int shell_help(char** args) {
  if (is_mounted == 0) {
    printf("This is a program simulate how ext2 file system works\n");
    printf(
        "You can use almost all the command as usual if you are not mount on "
        "the disk\n");
    printf("There are some built in command you can use:\n");
    printf("    mkdsk <path>\n");
    printf("    format <path>\n");
    printf("    mount <path>\n");
    printf("    help    show this information again\n");
    printf("    exit    quit this program\n");
  } else {
    printf("You have already mounted on ext2 file system!\n");
    printf("Following commands you can use:\n");
    printf("    mkdir <name>\n");
    printf("    touch <name>\n");
    printf("    chmod <name>\n");
    printf("    write <name>\n");
    printf("    cat <name>\n");
    printf("    pwd\n");
    printf("    ls\n");
    printf("    cd <path>\n");
    printf("    rm <name>\n");
    printf("    rmdir <name>\n");
    printf("    umount  unmount the file system\n");
    printf("    help    show this information again\n");
    printf("    exit    quit this program\n");
  }
  return 1;
}

int shell_clear(char** args) {
  shellLaunch(args);
  return 1;
}

int shell_exit(char** args) {
  printf("Bye!\n");
  exit(0);
}

int shellFuncNum() {
  return sizeof(commands) / sizeof(Command);
}

char* shellReadLine() {
  char* line = NULL;
  size_t bufsize = 0;  // 利用 getline 帮助我们分配缓冲区
  getline(&line, &bufsize, stdin);
  return line;
}

char** shellSplitLine(char* line) {
  int bufsize = TOK_BUFSIZE, position = 0;
  char** tokens = malloc(bufsize * sizeof(char*));
  char* token;

  if (!tokens) {
    fprintf(stderr, "shell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
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

  // 如果虚拟硬盘未挂载使用操作系统自带的命令
  if (!is_mounted) {
    for (i = 0; i < shellFuncNum(); i++) {
      if (strcmp(args[0], commands[i].name) == 0) {
        return (*commands[i].func)(args);
      }
    }
    return shellLaunch(args);
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

int shellLaunch(char** args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("shell");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("shell");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int shellLoop() {
  char* line;
  char** args;
  int status;

  do {
    char current_path[32] = {0};
    getCurrentPath(current_path);
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
  printf("Hello! Welcome to this toy EXT2 FILE SYSTEM\n");
  printf("Print \"help\" to see more information\n");
  stack_top = 0;
  path_stack[stack_top] = "/";
  shellLoop();
  exitDisplay();
}
