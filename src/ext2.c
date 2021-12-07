#include "ext2.h"

int checkExt2(char* path) {
  Disk disk;
  Ext2SuperBlock super_block;
  loadDisk(&disk, path);
  getSuperBlock(&disk, &super_block);
  if (super_block.magic == LINUX) {
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

int ext2Format(Disk* disk) {
  assert(disk != NULL);
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;

  // 初始化超级块和组描述符
  initSuperBlock(&super_block);
  initGdt(&gdt, &super_block);
  // 将超级块和组描述符，两个位图都写入
  writeSuperBlock(disk, &super_block);
  writeGdt(disk, &gdt);
  initInodeBitmap(disk);
  initBlockBitmap(disk);

  initRootDir(disk);

  printf("Successfully format the disk \"%s\" to Ext2\n", disk->path);
  printf("\nDisk Info:\n");
  printf("    Super Block Base:  %d\n", SUPER_BLOCK_BASE);
  printf("    GDT Block Base:    %d\n", GDT_BLOCK_BASE);
  printf("    Inode Bitmap Base: %d\n", INODE_BITMAP_BASE);
  printf("    Block Bitmap Base: %d\n", BLOCK_BITMAP_BASE);
  printf("    Inode Table Base:  %d\n", INODE_TABLE_BASE);
  printf("    Data Block Base:   %d\n", DATA_BLOCK_BASE);
  printf("    Free Blocks:       %d\n", super_block.free_blocks_count);
  printf("    Free Inodes:       %d\n", super_block.free_inodes_count);

  return SUCCESS;
}

int initSuperBlock(Ext2SuperBlock* super_block) {
  super_block->block_group = 0;
  super_block->blocks_count = NUMBER_OF_BLOCKS;
  super_block->blocks_per_group = NUMBER_OF_BLOCKS;
  super_block->inode_size = INODE_SIZE;
  super_block->first_data_block = SUPER_BLOCK_BASE;
  super_block->first_data_block_each_group = DATA_BLOCK_BASE;
  super_block->free_blocks_count = NUMBER_OF_BLOCKS - DATA_BLOCK_BASE;
  super_block->free_inodes_count = NUMBER_OF_INODES;
  super_block->magic = LINUX;
  super_block->first_ino = 11;
  super_block->errors = 0;
  super_block->log_block_size = 0;
  return SUCCESS;
}

int initGdt(Ext2GroupDescTable* gdt, Ext2SuperBlock* super_block) {
  Ext2GroupDesc gd;
  memset(gdt, 0, sizeof(Ext2GroupDescTable));
  memset(&gd, 0, GD_SIZE);

  gd.block_bitmap = BLOCK_BITMAP_BASE;
  gd.inode_bitmap = INODE_BITMAP_BASE;
  gd.free_blocks_count = super_block->free_blocks_count;
  gd.free_inodes_count = super_block->free_inodes_count;
  gd.used_dirs_count = 0;
  memcpy(&gdt->table[0], &gd, GD_SIZE);
  return SUCCESS;
}

int initInodeBitmap(Disk* disk) {
  BYTE bitmap[BLOCK_SIZE];
  memset(bitmap, 0, BLOCK_SIZE);

  writeInodeBitmap(disk, bitmap);

  return SUCCESS;
}

int initBlockBitmap(Disk* disk) {
  BYTE bitmap[BLOCK_SIZE];
  memset(bitmap, 0, BLOCK_SIZE);

  for (int i = 0; i < DATA_BLOCK_BASE; i++) {
    setBit(bitmap, i, 1);
  }

  writeBlockBitmap(disk, bitmap);

  return SUCCESS;
}

int initRootDir(Disk* disk) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  // 首先添加一个 inode
  Ext2Inode root_inode;
  root_inode.mode = 0x1FF | 0x4000;
  root_inode.size = 0;
  root_inode.blocks = 0;
  // inode 所处的位置
  Ext2Location root_inode_location;
  root_inode_location.block_idx = INODE_TABLE_BASE;
  root_inode_location.offset = 0;

  // 添加根目录，根目录的上级目录还是自己
  Ext2DirEntry entry;
  strcpy(entry.name, "..");
  entry.name_len = strlen("..");
  entry.file_type = EXT2_DIR;
  entry.rec_len = 2;
  entry.inode = 0;
  addDirEntry(disk, &root_inode, &entry);

  // 根目录的指向自己的目录块
  strcpy(entry.name, ".");
  entry.name_len = strlen(".");
  entry.file_type = EXT2_DIR;
  entry.rec_len = 2;
  entry.inode = 0;
  addDirEntry(disk, &root_inode, &entry);

  writeInode(disk, &root_inode, &root_inode_location);

  // 修改 Super Block 的值
  Ext2SuperBlock super_block;
  getSuperBlock(disk, &super_block);
  super_block.free_blocks_count--;
  writeSuperBlock(disk, &super_block);

  // 修改 Group Desc 的值
  Ext2GroupDescTable gdt;
  getGdt(disk, &gdt);
  gdt.table[0].free_blocks_count--;
  gdt.table[0].used_dirs_count++;
  gdt.table[0].free_inodes_count--;
  writeGdt(disk, &gdt);

  // 修改 block 位图
  setBlockBitmap(disk, DATA_BLOCK_BASE, 1);

  // 修改 inode 位图
  setInodeBitmap(disk, 0, 1);

  return SUCCESS;
}

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block) {
  unsigned int block_idx = SUPER_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memcpy(block, super_block, SUPER_BLOCK_SIZE);
  writeBlock(disk, block_idx, block);

  return SUCCESS;
}

int getSuperBlock(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memset(super_block, 0, sizeof(Ext2SuperBlock));
  readBlock(disk, SUPER_BLOCK_BASE, block);
  memcpy(super_block, block, sizeof(Ext2SuperBlock));
  return SUCCESS;
}

int writeGdt(Disk* disk, Ext2GroupDescTable* gdt) {
  unsigned int block_idx = GDT_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  memcpy(block, gdt, sizeof(Ext2GroupDescTable));
  writeBlock(disk, block_idx, block);
  return SUCCESS;
}

int getGdt(Disk* disk, Ext2GroupDescTable* gdt) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memset(gdt, 0, sizeof(Ext2GroupDescTable));
  readBlock(disk, GDT_BLOCK_BASE, block);
  memcpy(gdt, block, sizeof(Ext2GroupDescTable));
  return SUCCESS;
}

void getRootInode(Disk* disk, Ext2Inode* inode) {
  getInode(disk, 0, inode);
}

unsigned int getInodeIndex(Disk* disk, Ext2Inode* inode) {
  Ext2DirEntry entry;
  BYTE block[BLOCK_SIZE];
  unsigned int items = inode->size / DIR_SIZE;
  for (unsigned int i = 0; i < items; i++) {
    Ext2Location dir_location = getDirEntryLocation(disk, i, inode);
    readBlock(disk, dir_location.block_idx, block);
    memcpy(&entry, block + dir_location.offset, DIR_SIZE);
    if (!strcmp(entry.name, ".")) {
      return entry.inode;
    }
  }
  return 0;
}

int writeInode(Disk* disk, Ext2Inode* inode, Ext2Location* location) {
  BYTE block[BLOCK_SIZE];
  inode->mtime = time(NULL);
  readBlock(disk, location->block_idx, block);
  memcpy(block + location->offset, inode, INODE_SIZE);
  writeBlock(disk, location->block_idx, block);

  return SUCCESS;
}

int getInode(Disk* disk, unsigned int index, Ext2Inode* inode) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memset(inode, 0, INODE_SIZE);

  unsigned int inode_block = INODE_TABLE_BASE + index / INODES_PER_BLOCK;
  unsigned int inode_offset = (index % INODES_PER_BLOCK) * INODE_SIZE;

  readBlock(disk, inode_block, block);
  memcpy(inode, block + inode_offset, INODE_SIZE);
  return SUCCESS;
}

Ext2Location getFreeInode(Disk* disk) {
  Ext2Location location;
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;
  // 读取 inode 位图
  BYTE block[BLOCK_SIZE];
  readBlock(disk, INODE_BITMAP_BASE, block);
  for (int i = 0; i < NUMBER_OF_INODES / 8; i++) {
    if (block[i] != 0xff) {
      // 找到可用空位
      unsigned int offset = getOffset(block[i]);
      block[i] |= (0x80 >> offset);
      // 更新 inode 位图
      writeBlock(disk, INODE_BITMAP_BASE, block);
      // 修改文件系统信息
      // 更新 Super Block 信息
      getSuperBlock(disk, &super_block);
      super_block.free_inodes_count--;
      writeSuperBlock(disk, &super_block);
      // 更新 Group Desc 信息
      getGdt(disk, &gdt);
      gdt.table[0].free_inodes_count--;
      writeGdt(disk, &gdt);
      // 得到 inode 的序号
      unsigned int loc = i * 8 + offset;
      location.block_idx = INODE_TABLE_BASE + (loc * INODE_SIZE) / BLOCK_SIZE;
      location.offset = (loc * INODE_SIZE) % BLOCK_SIZE;
      return location;
    }
  }
  // 错误处理
  location.block_idx = -1;
  location.offset = -1;
  return location;
}

Ext2Location getFreeBlock(Disk* disk) {
  Ext2Location location;
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;
  // 读取 block 位图
  BYTE bitmap[BLOCK_SIZE];
  getBlockBitmap(disk, bitmap);
  for (int i = 0; i < NUMBER_OF_BLOCKS / 8; i++) {
    if (bitmap[i] != 0xff) {
      // 找到可用空位
      unsigned int offset = getOffset(bitmap[i]);
      bitmap[i] |= (0x80 >> offset);
      // 更新 block 位图
      writeBlockBitmap(disk, bitmap);
      // 修改文件系统信息
      // 更新 Super Block
      getSuperBlock(disk, &super_block);
      super_block.free_blocks_count--;
      writeSuperBlock(disk, &super_block);
      // 更新 Group Desc
      getGdt(disk, &gdt);
      gdt.table[0].free_blocks_count--;
      writeGdt(disk, &gdt);
      // 得到空闲 block 的序号
      unsigned int loc = i * 8 + offset;
      location.block_idx = DATA_BLOCK_BASE + loc;
      location.offset = 0;
      return location;
    }
  }
  // 错误处理
  location.block_idx = -1;
  location.offset = -1;
  return location;
}

int freeBlock(Disk* disk, int index) {
  // 删除 block 并更新 superblock 和 group desc
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;
  setBlockBitmap(disk, index, 0);
  getSuperBlock(disk, &super_block);
  getGdt(disk, &gdt);
  super_block.free_blocks_count++;
  gdt.table[0].free_blocks_count++;
  writeSuperBlock(disk, &super_block);
  writeGdt(disk, &gdt);
  return SUCCESS;
}

int freeInode(Disk* disk, int index) {
  // 删除 inode 并更新 superblock 和 group desc
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;
  setInodeBitmap(disk, index, 0);
  getSuperBlock(disk, &super_block);
  getGdt(disk, &gdt);
  super_block.free_inodes_count++;
  gdt.table[0].free_inodes_count++;
  writeSuperBlock(disk, &super_block);
  writeGdt(disk, &gdt);
  return SUCCESS;
}

Ext2Location getDirEntryLocation(Disk* disk,
                                 unsigned int index,
                                 Ext2Inode* parent) {
  UINT32* block = parent->block;
  Ext2Location location;
  BYTE tmp_block[BLOCK_SIZE];
  unsigned int num;
  unsigned int dir_block = index / DIRS_PER_BLOCK;
  unsigned int dir_offset = index % DIRS_PER_BLOCK;
  location.offset = dir_offset * DIR_SIZE;
  if (dir_block < 6) {
    // 直接寻址
    location.block_idx = block[dir_block];
    return location;
  } else {
    // 间接寻址
    dir_block = dir_block - 6;
    if (dir_block < BLOCK_SIZE / 4) {
      // 一级索引
      // 一个 block 512 字节，一个 int 4 个字节，所以一个 block 可以存储 128
      // 个 block
      readBlock(disk, block[6], tmp_block);
      memcpy(&num, tmp_block + dir_block * sizeof(unsigned int),
             sizeof(unsigned int));
      location.block_idx = num;
      return location;
    } else {
      // 二级索引
      dir_block = dir_block - BLOCK_SIZE / 4;
      unsigned int block_offset = dir_block / DIRS_PER_BLOCK;
      readBlock(disk, block[7], tmp_block);
      memcpy(&num, tmp_block + block_offset * sizeof(unsigned int),
             sizeof(unsigned int));
      readBlock(disk, num, tmp_block);
      memcpy(&num, tmp_block + dir_offset, sizeof(unsigned int));
      location.block_idx = num;
      return location;
    }
  }
}

int getDirEntry(Disk* disk,
                unsigned int index,
                Ext2Inode* parent,
                Ext2DirEntry* entry) {
  assert(entry != NULL);
  BYTE block[BLOCK_SIZE];
  Ext2Location dir_location = getDirEntryLocation(disk, index, parent);
  readBlock(disk, dir_location.block_idx, block);
  memcpy(entry, block + dir_location.offset, DIR_SIZE);
  return SUCCESS;
}

int writeDirEntry(Disk* disk,
                  unsigned int index,
                  Ext2Inode* parent,
                  Ext2DirEntry* entry) {
  assert(entry != NULL);
  BYTE block[BLOCK_SIZE];
  Ext2Location dir_location = getDirEntryLocation(disk, index, parent);
  readBlock(disk, dir_location.block_idx, block);
  memcpy(block + dir_location.offset, entry, DIR_SIZE);
  writeBlock(disk, dir_location.block_idx, block);
  return SUCCESS;
}

int getCurrentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry) {
  return getDirEntry(disk, 1, inode, entry);
}
int getParentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry) {
  return getDirEntry(disk, 0, inode, entry);
}
int writeCurrentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry) {
  return writeDirEntry(disk, 1, inode, entry);
}
int writeParentEntry(Disk* disk, Ext2Inode* inode, Ext2DirEntry* entry) {
  return writeDirEntry(disk, 0, inode, entry);
}

int writeFile(Disk* disk, Ext2Inode* inode) {
  printf("Please input something, type <Esc> to stop writing.\n");
  BYTE buffer[BLOCK_SIZE];
  memset(buffer, 0, BLOCK_SIZE);
  int cursor = 0;

  char str = getCh();
  while (str != 27) {
    printf("%c", str);
    // if (!(inode->size %512) {
    // TODO 扩容
    // }
    memcpy(buffer + (cursor++), &str, sizeof(char));
    if (str == 0x0d)
      printf("%c", 0x0a);
    str = getCh();
    if (str == 27)
      break;
  }
  printf("\n");

  // 将block加入 inode
  Ext2Location location = getFreeBlock(disk);
  writeBlock(disk, location.block_idx, buffer);
  inode->blocks++;
  inode->block[0] = location.block_idx;
  inode->size = cursor;
  inode->mtime = time(NULL);
  return SUCCESS;
}

int readFile(Disk* disk, Ext2Inode* inode) {
  BYTE block[BLOCK_SIZE];
  if (inode->size == 0) {
    // 文件为空
    printf("%%empty%%\n");
    return SUCCESS;
  }
  for (int i = 0; i < inode->blocks; i++) {
    // 将所有 block 中的内容输出
    Ext2Location block_loc = getDirEntryLocation(disk, i * 16, inode);
    readBlock(disk, block_loc.block_idx, block);
    printf("%s", block);
  }
  printf("\n");
  return SUCCESS;
}

unsigned int addDirEntry(Disk* disk,
                         Ext2Inode* parent_inode,
                         Ext2DirEntry* entry) {
  BYTE block[BLOCK_SIZE];
  unsigned int total = parent_inode->size / DIR_SIZE;
  unsigned int dir_block = total / DIRS_PER_BLOCK;
  unsigned int dir_offset = total % DIRS_PER_BLOCK;
  if (dir_block < 6) {
    // 直接索引
    if (parent_inode->blocks < dir_block + 1) {
      // 如果没有创建新的 block
      Ext2Location block_location = getFreeBlock(disk);
      parent_inode->block[dir_block] = block_location.block_idx;
      parent_inode->blocks++;
    }
    readBlock(disk, parent_inode->block[dir_block], block);
    memcpy(block + dir_offset * DIR_SIZE, entry, DIR_SIZE);
    writeBlock(disk, parent_inode->block[dir_block], block);
    parent_inode->size += DIR_SIZE;
    return SUCCESS;
  } else {
    // 间接索引
    // 如果没有创建新的 block
    if (parent_inode->blocks < 7) {
      Ext2Location block_location = getFreeBlock(disk);
      parent_inode->block[6] = block_location.block_idx;
      parent_inode->blocks++;
    }
    dir_block = dir_block - 6;
    unsigned int dir_records_per_block = BLOCK_SIZE / sizeof(unsigned int);
    if (dir_block < dir_records_per_block) {
      // 一级索引
      readBlock(disk, parent_inode->block[6], block);
      if (parent_inode->blocks < dir_block + 6 + 2) {
        // 为索引块添加一个 block
        Ext2Location block_location = getFreeBlock(disk);
        memcpy(block + dir_block * sizeof(int), &block_location.block_idx,
               sizeof(int));
        writeBlock(disk, parent_inode->block[6], block);
        parent_inode->blocks++;
      }
      unsigned int block_idx;
      memcpy(&block_idx, block + dir_block * sizeof(int), sizeof(int));
      BYTE block1[BLOCK_SIZE];
      readBlock(disk, block_idx, block1);
      memcpy(block1 + dir_offset * DIR_SIZE, entry, DIR_SIZE);
      writeBlock(disk, block_idx, block1);
      parent_inode->size += DIR_SIZE;
      return SUCCESS;
    } else {
      // 没有初始化二级索引则初始化 block
      if (parent_inode->blocks < total + 1) {
        Ext2Location block_location = getFreeBlock(disk);
        parent_inode->block[7] = block_location.block_idx;
      }
      // 二级索引
      dir_block = dir_block - dir_records_per_block;
      unsigned block_block = dir_block / dir_records_per_block;
      unsigned block_offset = dir_block % dir_records_per_block;
      unsigned int num;
      // TODO 完成二级索引
      readBlock(disk, parent_inode->block[7], block);
      memcpy(&num, block + block_block * sizeof(unsigned int),
             sizeof(unsigned int));
      readBlock(disk, DATA_BLOCK_BASE + num, block);
      memcpy(block + dir_offset * DIR_SIZE, &entry, DIR_SIZE);
      parent_inode->size += DIR_SIZE;
      return FAILURE;
      return SUCCESS;
    }
  }
}

int ext2Ls(Ext2FileSystem* file_system, Ext2Inode* current) {
  unsigned int items = current->size / DIR_SIZE;
  Ext2DirEntry dir;
  // 读取 current 对应第一块 block
  printf(
      "\x1B[4mType\x1B[0m\t\x1B[4mPermission\x1B[0m\t\x1B[4mSize\x1B[0m\t\x1B["
      "4mModify Time\x1B[0m\t\t\t\x1B[4mName\x1B[0m\t\n");
  BYTE block[BLOCK_SIZE];
  for (unsigned int i = 0; i < items; i++) {
    Ext2Location location = getDirEntryLocation(file_system->disk, i, current);
    readBlock(file_system->disk, location.block_idx, block);
    memcpy(&dir, block + location.offset, DIR_SIZE);
    Ext2Inode temp;
    getInode(file_system->disk, dir.inode, &temp);
    char str_type[16];
    char str_permission[16];
    char str_size[16];
    char str_time[128];
    char str_name[16];
    strcpy(str_name, dir.name);
    if (dir.file_type == EXT2_DIR) {
      strcpy(str_type, "Dir");
      strcpy(str_permission, "-\t");
      strcpy(str_size, "-");
    } else if (dir.file_type == EXT2_FILE) {
      strcpy(str_type, "File");
      int permission = (temp.mode >> 1);
      if (permission == WRITABLE) {
        strcpy(str_permission, "Writable");
      } else {
        strcpy(str_permission, "Can't Write");
      }
      sprintf(str_size, "%d", temp.size);
    }
    strcpy(str_time, "");
    strcat(str_time, asctime(localtime(&temp.mtime)));
    for (int j = 0; j < strlen(str_time); j++) {
      if (str_time[j] == '\n') {
        str_time[j] = '\t';
      }
    }
    printf("%s\t%s\t%s\t%s%s\n", str_type, str_permission, str_size, str_time,
           str_name);
  }
  return SUCCESS;
}

int ext2Mount(Ext2FileSystem* file_system, Ext2Inode* current, char* path) {
  if (file_system->disk == NULL) {
    file_system->disk = (Disk*)malloc(sizeof(Disk));
  }
  // 挂载磁盘
  loadDisk(file_system->disk, path);
  // 得到根路径
  getRootInode(file_system->disk, current);
  return SUCCESS;
}

int ext2Mkdir(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  if (strlen(name) >= DIR_NAME_LEN) {
    printf(
        "Warring! Too large name length, appropriate name length is below %d\n",
        DIR_NAME_LEN);
    return FAILURE;
  }
  // 查询是否已经存在同名文件
  Ext2DirEntry entry;
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 2; i < items; i++) {
    getDirEntry(file_system->disk, i, current, &entry);
    if (!strcmp(entry.name, name)) {
      // 存在同名文件
      printf("There are already a file or directory named %s\n", name);
      return FAILURE;
    }
  }

  // 没有同名文件或文件夹，新建一个 inode
  Ext2Location inode_location = getFreeInode(file_system->disk);
  // 空闲 inode 的序号
  unsigned int inode_idx =
      (inode_location.block_idx - INODE_TABLE_BASE) * INODES_PER_BLOCK +
      inode_location.offset / INODE_SIZE;

  // 将新的目录项添加到父目录下
  strcpy(entry.name, name);
  entry.inode = inode_idx;
  entry.name_len = strlen(name);
  entry.file_type = EXT2_DIR;
  entry.rec_len = 2;
  addDirEntry(file_system->disk, current, &entry);
  // 获得当前目录的 Dir Entry
  Ext2DirEntry parent_entry;
  getCurrentEntry(file_system->disk, current, &parent_entry);
  parent_entry.rec_len++;
  writeCurrentEntry(file_system->disk, current, &parent_entry);

  // 写入新目录的 inode
  Ext2Inode new_inode;
  memset(&new_inode, 0, INODE_SIZE);
  new_inode.mode = EXT2_DIR;
  new_inode.blocks = 0;
  new_inode.size = 0;

  // 写入 dir entry
  // 写入上级目录
  strcpy(parent_entry.name, "..");
  addDirEntry(file_system->disk, &new_inode, &parent_entry);

  // 写入当前目录
  strcpy(entry.name, ".");
  addDirEntry(file_system->disk, &new_inode, &entry);

  writeInode(file_system->disk, &new_inode, &inode_location);

  // 更新 current
  unsigned int parent_inode_index = parent_entry.inode;
  Ext2Location parent_location;
  parent_location.block_idx =
      INODE_TABLE_BASE + parent_inode_index / INODES_PER_BLOCK;
  parent_location.offset = (parent_inode_index % INODES_PER_BLOCK) * INODE_SIZE;
  writeInode(file_system->disk, current, &parent_location);

  return SUCCESS;
}

int ext2Touch(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  if (strlen(name) >= DIR_NAME_LEN) {
    printf(
        "Warring! Too large name length, appropriate name length is below %d\n",
        DIR_NAME_LEN);
    return FAILURE;
  }
  // 查询是否已经存在同名文件
  Ext2DirEntry entry;
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 2; i < items; i++) {
    getDirEntry(file_system->disk, i, current, &entry);
    if (!strcmp(entry.name, name)) {
      // 存在同名文件
      printf("There are already a file or directory named %s\n", name);
      return FAILURE;
    }
  }

  // 没有同名文件或文件夹，新建一个 inode
  Ext2Location inode_location = getFreeInode(file_system->disk);
  // 空闲 inode 的序号
  unsigned int inode_idx =
      (inode_location.block_idx - INODE_TABLE_BASE) * INODES_PER_BLOCK +
      inode_location.offset / INODE_SIZE;

  // 将新的目录项添加到父目录下
  strcpy(entry.name, name);
  entry.inode = inode_idx;
  entry.name_len = strlen(name);
  entry.file_type = EXT2_FILE;
  entry.rec_len = 0;
  addDirEntry(file_system->disk, current, &entry);
  // 获得当前目录的 Dir Entry
  Ext2DirEntry parent_entry;
  getCurrentEntry(file_system->disk, current, &parent_entry);
  parent_entry.rec_len++;
  writeCurrentEntry(file_system->disk, current, &parent_entry);

  // 写入新目录的 inode
  Ext2Inode new_inode;
  memset(&new_inode, 0, INODE_SIZE);
  new_inode.mode = EXT2_DIR + (WRITABLE << 1);
  new_inode.blocks = 0;
  new_inode.size = 0;
  writeInode(file_system->disk, &new_inode, &inode_location);

  // 更新 current
  unsigned int parent_inode_index = parent_entry.inode;
  Ext2Location parent_location;
  parent_location.block_idx =
      INODE_TABLE_BASE + parent_inode_index / INODES_PER_BLOCK;
  parent_location.offset = (parent_inode_index % INODES_PER_BLOCK) * INODE_SIZE;
  writeInode(file_system->disk, current, &parent_location);

  return SUCCESS;
}

int ext2Chmod(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  // 先找到这个文件入口
  Ext2DirEntry entry;
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 2; i < items; i++) {
    getDirEntry(file_system->disk, i, current, &entry);
    if (!strcmp(entry.name, name)) {
      // 找到同名的 Dir Entry
      if (entry.file_type != EXT2_FILE) {
        // 不是文件
        printf("This is a directory!\n");
        return FAILURE;
      }
      break;
    }
  }
  Ext2Inode inode;
  getInode(file_system->disk, entry.inode, &inode);
  if (inode.mode == WRITABLE) {
    inode.mode = 0;
  } else {
    inode.mode = 2;
  }
  Ext2Location location;
  location.block_idx = INODE_TABLE_BASE + entry.inode / INODES_PER_BLOCK;
  location.offset = (entry.inode % INODES_PER_BLOCK) * INODE_SIZE;
  writeInode(file_system->disk, &inode, &location);
  return SUCCESS;
}

int ext2Rmdir(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  return deleteDirEntry(file_system, current, name, EXT2_DIR);
}

int ext2Rm(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  return deleteDirEntry(file_system, current, name, EXT2_FILE);
}

int deleteDirEntry(Ext2FileSystem* file_system,
                   Ext2Inode* current,
                   char* name,
                   int type) {
  // 删除文件或文件夹，使用递归的方法：
  // 1. 如果删除的是文件，则在当前文件夹下删除该文件的 dir entry，然后删除文件的
  // inode,并将该 inode 下的所有 block 都释放
  // 2. 如果删除的是文件夹，则在当前文件夹下删除该目录的 dir
  // entry，然后遍历文件夹下的所有文件进行删除，并再次递归删除文件夹下的所有文件夹
  BYTE block[BLOCK_SIZE];
  // 无法删除上级目录和当前目录
  if (!strcmp(name, ".") || !strcmp(name, "..")) {
    printf("Error : can't delete current work directory!\n");
    return FAILURE;
  }
  // 寻找文件/目录的 Dir Entry
  Ext2DirEntry entry;  // 要删除的 Dir Entry
  int entry_index;
  Ext2DirEntry last_entry;  // 当前目录最后的 Dir Entry
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 2; i < items; i++) {
    getDirEntry(file_system->disk, i, current, &last_entry);
    if (!strcmp(last_entry.name, name)) {
      // 找到同名的 Dir Entry
      if (last_entry.file_type != type) {
        // 类型不同，删除失败
        switch (last_entry.file_type) {
          case EXT2_DIR:
            printf("This is directory, please use \"rmdir\" to delete!\n");
            return FAILURE;
          case EXT2_FILE:
            printf("This is a file, please use \"rm\" to delete!\n");
            return FAILURE;
          default:
            printf("Error : Invalid file type!\n");
            return FAILURE;
        }
      }
      // 记录待删除 Dir Entry 的位置
      memcpy(&entry, &last_entry, DIR_SIZE);
      entry_index = i;
    }
  }

  // * 先删除当前目录的信息
  // 将 inode->block 中最后的 entry 与待删除的 entry 互换位置
  if (entry_index != (items - 1)) {
    // 待删除的 entry 不是末尾
    Ext2Location entry_loc;
    entry_loc = getDirEntryLocation(file_system->disk, entry_index, current);
    Ext2DirEntry temp;
    readBlock(file_system->disk, entry_loc.block_idx, block);
    memcpy(block + entry_loc.offset, &last_entry, DIR_SIZE);
    writeBlock(file_system->disk, entry_loc.block_idx, block);
  }
  Ext2Location last_location;  // 最后 entry 的位置
  last_location = getDirEntryLocation(file_system->disk, items - 1, current);
  if (last_location.offset == 0) {
    // 在新块
    freeBlock(file_system->disk, last_location.block_idx);
  }
  current->size -= DIR_SIZE;
  // 将 current 更新到 disk
  Ext2DirEntry current_entry;
  getDirEntry(file_system->disk, 1, current, &current_entry);
  int current_inode_index = current_entry.inode;
  Ext2Location loc;
  loc.block_idx = INODE_TABLE_BASE + current_inode_index / INODES_PER_BLOCK;
  loc.offset = (current_inode_index % INODES_PER_BLOCK) * INODE_SIZE;
  writeInode(file_system->disk, current, &loc);

  // * 再处理待删除的 inode
  int inode_idx = entry.inode;
  Ext2Inode inode;
  getInode(file_system->disk, inode_idx, &inode);
  if (type == EXT2_DIR) {
    // 如果删除的是文件夹
    if (inode.size == DIR_SIZE * 2) {
      // 空文件夹，直接删除
      freeBlock(file_system->disk, inode.block[0]);
      freeInode(file_system->disk, inode_idx);
      return SUCCESS;
    }
    // 文件夹非空，遍历删除
    for (int ii = 2; ii < inode.size / DIR_SIZE; ii++) {
      Ext2DirEntry child_entry;
      getDirEntry(file_system->disk, ii, &inode, &child_entry);
      if (child_entry.file_type == EXT2_DIR) {
        ext2Rmdir(file_system, &inode, child_entry.name);
      } else {
        ext2Rm(file_system, &inode, child_entry.name);
      }
      // 删除当前 inode
      freeInode(file_system->disk, inode_idx);
      return SUCCESS;
    }
  } else {
    // 如果删除的是文件
    for (int ii = 2; ii < inode.blocks; ii++) {
      // 将文件的 block 都删除
      // 先删除文件的直接索引
      Ext2Location location =
          getDirEntryLocation(file_system->disk, ii * 12, &inode);
      freeBlock(file_system->disk, location.block_idx);
      setBlockBitmap(file_system->disk, location.block_idx, 0);
      // TODO 删除所有一级索引
      // TODO 删除所有二级索引
      // 删除当前 inode
      freeInode(file_system->disk, inode_idx);
      return SUCCESS;
    }
  }

  return FAILURE;
}

int ext2Open(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  Ext2DirEntry entry;
  BYTE block[BLOCK_SIZE];

  // 当前目录的文件项数
  unsigned int items = current->size / DIR_SIZE;
  for (int i = 0; i < items; i++) {
    // 得到第 i 个 Dir Entry 的绝对位置
    Ext2Location dir_location =
        getDirEntryLocation(file_system->disk, i, current);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(&entry, block + dir_location.offset, DIR_SIZE);
    if (!strcmp(entry.name, name)) {
      if (entry.file_type == EXT2_FILE) {
        printf("It's not a directory!\n");
        return FAILURE;
      }
      Ext2Inode root_inode;
      readBlock(file_system->disk, INODE_TABLE_BASE, block);
      memcpy(&root_inode, block, INODE_SIZE);
      getInode(file_system->disk, entry.inode, current);
      return SUCCESS;
    }
  }
  printf("There's no directory named \"%s\"\n", name);
  return FAILURE;
}

int ext2Write(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  // 先找到这个文件入口
  Ext2DirEntry entry;
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 2; i < items; i++) {
    getDirEntry(file_system->disk, i, current, &entry);
    if (!strcmp(entry.name, name)) {
      // 找到同名的 Dir Entry
      if (entry.file_type != EXT2_FILE) {
        // 不是文件
        printf("This is a directory!\n");
        return FAILURE;
      }
      break;
    }
  }

  if (strcmp(entry.name, name)) {
    // 文件不存在
    if (ext2Touch(file_system, current, name) == FAILURE) {
      return FAILURE;
    }
    // 找到这个文件入口
    unsigned int items = current->size / DIR_SIZE;
    for (unsigned int i = 2; i < items; i++) {
      getDirEntry(file_system->disk, i, current, &entry);
      if (!strcmp(entry.name, name)) {
        break;
      }
    }
  }

  // 找到 entry 后
  Ext2Inode inode;
  getInode(file_system->disk, entry.inode, &inode);
  if ((inode.mode >> 1) == CANNOT_WRITE) {
    printf("Permission denied. Please use \"chmod\" to write this file.\n");
    return FAILURE;
  }
  writeFile(file_system->disk, &inode);
  Ext2Location loc;
  loc.block_idx = INODE_TABLE_BASE + entry.inode / INODES_PER_BLOCK;
  loc.offset = (entry.inode % INODES_PER_BLOCK) * INODE_SIZE;
  writeInode(file_system->disk, &inode, &loc);

  return SUCCESS;
}

int ext2Cat(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  // 先找到这个文件入口
  Ext2DirEntry entry;
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 2; i < items; i++) {
    getDirEntry(file_system->disk, i, current, &entry);
    if (!strcmp(entry.name, name)) {
      // 找到同名的 Dir Entry
      if (entry.file_type != EXT2_FILE) {
        // 不是文件
        printf("This is a directory!\n");
        return FAILURE;
      }
      break;
    }
  }

  if (strcmp(entry.name, name)) {
    // 文件不存在
    printf("The file named \"%s\" isn't exist\n", name);
    return FAILURE;
  }

  // 找到 entry 后
  Ext2Inode inode;
  getInode(file_system->disk, entry.inode, &inode);
  readFile(file_system->disk, &inode);
  return SUCCESS;
}

void getInodeBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]) {
  memset(bitmap, 0, BLOCK_SIZE);
  readBlock(disk, INODE_BITMAP_BASE, bitmap);
}

void getBlockBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]) {
  memset(bitmap, 0, BLOCK_SIZE);
  readBlock(disk, BLOCK_BITMAP_BASE, bitmap);
}

void setInodeBitmap(Disk* disk, unsigned int index, int value) {
  BYTE bitmap[BLOCK_SIZE];
  getInodeBitmap(disk, bitmap);
  setBit(bitmap, index, value);
  writeInodeBitmap(disk, bitmap);
}

void setBlockBitmap(Disk* disk, unsigned int index, int value) {
  BYTE bitmap[BLOCK_SIZE];
  getBlockBitmap(disk, bitmap);
  setBit(bitmap, index, value);
  writeBlockBitmap(disk, bitmap);
}

void writeInodeBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]) {
  writeBlock(disk, INODE_BITMAP_BASE, bitmap);
}

void writeBlockBitmap(Disk* disk, BYTE bitmap[BLOCK_SIZE]) {
  writeBlock(disk, BLOCK_BITMAP_BASE, bitmap);
}

int printDiskInfo(Disk* disk) {
  Ext2SuperBlock super_block;
  getSuperBlock(disk, &super_block);
  printf("Disk Info:\n");
  printf("    Inode size: %d bytes\n", INODE_SIZE);
  printf("    Block size: %d bytes\n", BLOCK_SIZE);
  printf("    Inodes count: %d\n", super_block.inodes_count);
  printf("    Blocks count: %d\n", super_block.blocks_count);
  printf("    Free Inodes: %d\n", super_block.free_inodes_count);
  printf("    Free Blocks: %d\n", super_block.free_blocks_count);
  return SUCCESS;
}

int setBit(BYTE bitmap[BLOCK_SIZE], int index, int value) {
  int byte = index / 8;
  int offset = index % 8;

  if (value == 0)
    bitmap[byte] ^= (0x1 << (7 - offset));
  else
    bitmap[byte] |= (0x1 << (7 - offset));
  return SUCCESS;
}

int getOffset(BYTE byte) {
  if (byte == 0xff)
    return -1;
  int offset = 0;
  while (1) {
    if ((byte >> (7 - offset)) % 2 == 0) {
      return offset;
    } else {
      offset++;
    }
  }
}

int writeBlock(Disk* disk, unsigned int block_idx, void* block) {
  disk->write_disk(disk, block_idx, block);
  return SUCCESS;
}

int readBlock(Disk* disk, unsigned int block_idx, void* block) {
  disk->read_disk(disk, block_idx, block);
  return SUCCESS;
}

char getCh() {
  char ch;
  struct termios old_t, new_t;
  tcgetattr(STDIN_FILENO, &old_t);
  new_t = old_t;
  new_t.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_t);
  ch = getchar();
  tcsetattr(STDIN_FILENO, TCSANOW, &old_t);
  return ch;
}
