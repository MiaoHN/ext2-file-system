#include "ext2.h"

int ext2Format(Disk *disk) {
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

  return SUCCESS;
}

int initSuperBlock(Ext2SuperBlock *super_block) {
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

int initGdt(Ext2GroupDescTable *gdt, Ext2SuperBlock *super_block) {
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

int initInodeBitmap(Disk *disk) {
  BYTE bitmap[BLOCK_SIZE];
  memset(bitmap, 0, BLOCK_SIZE);

  writeInodeBitmap(disk, bitmap);

  return SUCCESS;
}

int initBlockBitmap(Disk *disk) {
  BYTE bitmap[BLOCK_SIZE];
  memset(bitmap, 0, BLOCK_SIZE);

  for (int i = 0; i < DATA_BLOCK_BASE; i++) {
    setBit(bitmap, i, 1);
  }

  writeBlockBitmap(disk, bitmap);

  return SUCCESS;
}

int initRootDir(Disk *disk) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  // 首先添加一个 inode
  Ext2Inode root_inode;
  root_inode.mode = 0x1FF | 0x4000;
  root_inode.size = 0;  // 根目录有两个初始文件
  root_inode.blocks = 0;
  // inode 所处的位置
  Ext2Location root_inode_location;
  root_inode_location.block_idx = INODE_TABLE_BASE;
  root_inode_location.offset = 0;

  // 添加根目录，根目录的上级目录还是自己
  Ext2DirEntry entry;
  strcpy(entry.name, "..");
  entry.name_len = strlen("..");
  entry.file_type = DIR_TYPE;
  entry.inode = 0;
  addDirEntry(disk, &root_inode, &entry);

  // 根目录的指向自己的目录块
  strcpy(entry.name, ".");
  entry.name_len = strlen(".");
  entry.file_type = DIR_TYPE;
  entry.inode = 0;
  addDirEntry(disk, &root_inode, &entry);

  addInode(disk, &root_inode, &root_inode_location);

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

int writeSuperBlock(Disk *disk, Ext2SuperBlock *super_block) {
  unsigned int block_idx = SUPER_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memcpy(block, super_block, SUPER_BLOCK_SIZE);
  writeBlock(disk, block_idx, block);

  return SUCCESS;
}

int getSuperBlock(Disk *disk, Ext2SuperBlock *super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memset(super_block, 0, sizeof(Ext2SuperBlock));
  readBlock(disk, SUPER_BLOCK_BASE, block);
  memcpy(super_block, block, sizeof(Ext2SuperBlock));
  return SUCCESS;
}

int writeGdt(Disk *disk, Ext2GroupDescTable *gdt) {
  unsigned int block_idx = GDT_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  memcpy(block, gdt, sizeof(Ext2GroupDescTable));
  writeBlock(disk, block_idx, block);
  return SUCCESS;
}

int getGdt(Disk *disk, Ext2GroupDescTable *gdt) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memset(gdt, 0, sizeof(Ext2GroupDescTable));
  readBlock(disk, GDT_BLOCK_BASE, block);
  memcpy(gdt, block, sizeof(Ext2GroupDescTable));
  return SUCCESS;
}

void getRootInode(Disk *disk, Ext2Inode *inode) { getInode(disk, 0, inode); }

int addInode(Disk *disk, Ext2Inode *inode, Ext2Location *location) {
  BYTE block[BLOCK_SIZE];
  readBlock(disk, location->block_idx, block);
  memcpy(block + location->offset, inode, INODE_SIZE);
  writeBlock(disk, location->block_idx, block);

  return SUCCESS;
}

int getInode(Disk *disk, unsigned int index, Ext2Inode *inode) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memset(inode, 0, INODE_SIZE);

  unsigned int inode_block = INODE_TABLE_BASE + index / INODES_PER_BLOCK;
  unsigned int inode_offset = (index % INODES_PER_BLOCK) * INODE_SIZE;

  readBlock(disk, inode_block, block);
  memcpy(inode, block + inode_offset, INODE_SIZE);
}

Ext2Location getFreeInode(Disk *disk) {
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

Ext2Location getFreeBlock(Disk *disk) {
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

Ext2Location getDirEntry(Disk *disk, unsigned int index,
                         unsigned int block[8]) {
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

unsigned int addDirEntry(Disk *disk, Ext2Inode *inode, Ext2DirEntry *entry) {
  BYTE block[BLOCK_SIZE];
  unsigned int total = inode->size / DIR_SIZE;
  unsigned int dir_block = total / DIRS_PER_BLOCK;
  unsigned int dir_offset = total % DIRS_PER_BLOCK;
  if (dir_block < 6) {
    // 直接索引
    if (dir_offset == 0 && dir_block != 0) {
      // 刚好在上一个 block 中
      dir_block -= 1;
    } else if (inode->blocks < dir_block + 1) {
      // 如果没有创建新的 block
      Ext2Location block_location = getFreeBlock(disk);
      inode->block[dir_block] = block_location.block_idx;
      inode->blocks++;
    }
    readBlock(disk, inode->block[dir_block], block);
    memcpy(block + dir_offset * DIR_SIZE, entry, DIR_SIZE);
    writeBlock(disk, inode->block[dir_block], block);
    inode->size += DIR_SIZE;
    return SUCCESS;
  } else {
    // 间接索引
    if (dir_block == 6 && dir_offset == 0) {
      // 刚好在上一个 block 中，直接写入
      readBlock(disk, inode->block[5], block);
      memcpy(block + (DIRS_PER_BLOCK - 1) * DIR_SIZE, &entry, DIR_SIZE);
      writeBlock(disk, inode->block[5], block);
      inode->size += DIR_SIZE;
      return SUCCESS;
    }
    // 如果没有创建新的 block
    if (inode->blocks < total + 1) {
      Ext2Location block_location = getFreeBlock(disk);
      inode->block[6] = block_location.block_idx;
      inode->blocks++;
    }
    dir_block = dir_block - 6;
    unsigned int dir_records_per_block = BLOCK_SIZE / sizeof(unsigned int);
    if (dir_block < dir_records_per_block) {
      // 一级索引
      readBlock(disk, inode->block[6], block);
      memcpy(block + dir_offset * DIR_SIZE, &entry, DIR_SIZE);
      writeBlock(disk, inode->block[dir_block], block);
      inode->size += DIR_SIZE;
      return SUCCESS;
    } else {
      if (dir_block == dir_records_per_block && dir_offset == 0) {
        // 刚好在一级索引末尾
        readBlock(disk, inode->block[6], block);
        memcpy(block + (dir_records_per_block - 1) * DIR_SIZE, &entry,
               DIR_SIZE);
        writeBlock(disk, inode->block[dir_block], block);
        inode->size += DIR_SIZE;
        return SUCCESS;
      }
      // 没有初始化二级索引则初始化 block
      if (inode->blocks < total + 1) {
        Ext2Location block_location = getFreeBlock(disk);
        inode->block[7] = block_location.block_idx;
      }
      // 二级索引
      dir_block = dir_block - dir_records_per_block;
      unsigned block_block = dir_block / dir_records_per_block;
      unsigned block_offset = dir_block % dir_records_per_block;
      unsigned int num;
      // TODO
      readBlock(disk, inode->block[7], block);
      memcpy(&num, block + block_block * sizeof(unsigned int),
             sizeof(unsigned int));
      readBlock(disk, DATA_BLOCK_BASE + num, block);
      memcpy(block + dir_offset * DIR_SIZE, &entry, DIR_SIZE);
      inode->size += DIR_SIZE;
      return FAILURE;
      return SUCCESS;
    }
  }
}

int ext2Ls(Ext2FileSystem *file_system, Ext2Inode *current) {
  unsigned int items = current->size / DIR_SIZE;
  Ext2DirEntry dir;
  // 读取 current 对应第一块 block
  printf("Type\t\tName\tInode\n");
  BYTE block[BLOCK_SIZE];
  for (unsigned int i = 0; i < items; i++) {
    Ext2Location location = getDirEntry(file_system->disk, i, current->block);
    readBlock(file_system->disk, location.block_idx, block);
    memcpy(&dir, block + location.offset, DIR_SIZE);
    if (dir.file_type == EXT2_DIR) {
      printf("Dir\t\t\t");
    } else if (dir.file_type == EXT2_FILE) {
      printf("File\t\t\t");
    } else {
      printf("error occurred!\n");
      return FAILURE;
    }
    printf("%s\t\t%d\n", dir.name, dir.inode);
  }
  return SUCCESS;
}

int ext2Mount(Ext2FileSystem *file_system, Ext2Inode *current, char *path) {
  if (file_system->disk == NULL) {
    file_system->disk = (Disk *)malloc(sizeof(Disk));
  }
  // 挂载磁盘
  loadDisk(file_system->disk, path);
  // 得到根路径
  getRootInode(file_system->disk, current);
  return SUCCESS;
}

int ext2Mkdir(Ext2FileSystem *file_system, Ext2Inode *current, char *name) {
  Ext2DirEntry *entry;
  entry = (Ext2DirEntry *)malloc(DIR_SIZE);
  BYTE block[BLOCK_SIZE];

  // 查询是否已经存在同名文件
  unsigned int items = current->size / DIR_SIZE;
  for (unsigned int i = 0; i < items; i++) {
    Ext2Location dir_location =
        getDirEntry(file_system->disk, i, current->block);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(entry, block + dir_location.offset, DIR_SIZE);
    if (!strcmp(entry->name, name)) {
      // 存在同名文件
      printf("There are already a file or directory named %s\n", name);
      return FAILURE;
    }
  }

  // 没有同名文件或文件夹，新建一个 inode 和对应的 data block
  Ext2Location inode_location = getFreeInode(file_system->disk);
  Ext2Location block_location = getFreeBlock(file_system->disk);
  // 空闲 inode 的序号
  unsigned int inode_idx =
      (inode_location.block_idx - INODE_TABLE_BASE) * INODES_PER_BLOCK +
      inode_location.offset / INODE_SIZE;

  // 获得当前目录的 inode 值
  entry = (Ext2DirEntry *)block;
  // '.'目录初始化在第一块处
  readBlock(file_system->disk, current->block[0], block);
  while (strcmp(entry->name, ".") != 0) {
    entry++;
  }
  unsigned int parent_inode = entry->inode;

  // 将新的目录项添加到当前目录下
  strcpy(entry->name, name);
  entry->inode = inode_idx;
  entry->name_len = strlen(name);
  entry->file_type = EXT2_DIR;
  entry->rec_len = 2;
  addDirEntry(file_system->disk, current, entry);

  // 写入新目录的 inode
  Ext2Inode new_inode;
  memset(&new_inode, 0, INODE_SIZE);
  new_inode.mode = 2;
  new_inode.blocks = 1;           // 当前和上一层目录
  new_inode.size = DIR_SIZE * 2;  // 两个文件夹
  new_inode.block[0] = block_location.block_idx;
  addInode(file_system->disk, &new_inode, &inode_location);

  // 写入 dir entry
  memset(block, 0, BLOCK_SIZE);
  entry = (Ext2DirEntry *)block;
  // 写入上级目录
  entry->inode = parent_inode;
  entry->rec_len = DIR_SIZE;
  entry->file_type = EXT2_DIR;
  entry->name_len = 2;
  strcpy(entry->name, "..");
  entry->file_type = EXT2_DIR;
  // 写入当前目录
  entry++;
  entry->inode = inode_idx;
  entry->rec_len = DIR_SIZE;
  entry->file_type = EXT2_DIR;
  entry->name_len = 1;
  strcpy(entry->name, ".");
  entry->file_type = EXT2_DIR;
  // 写入磁盘
  writeBlock(file_system->disk, block_location.block_idx, block);

  return SUCCESS;
}

int ext2Touch(Ext2FileSystem *file_system, Ext2Inode *current, char *name) {
  Ext2DirEntry *entry = (Ext2DirEntry *)malloc(DIR_SIZE);
  BYTE block[BLOCK_SIZE];

  // 查询是否已经存在同名文件
  unsigned int items = current->size / DIR_SIZE;
  for (int i = 0; i < items; i++) {
    Ext2Location dir_location =
        getDirEntry(file_system->disk, i, current->block);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(entry, block + dir_location.offset, DIR_SIZE);
    if (!strcmp(entry->name, name)) {
      // 存在同名文件
      printf("There are already a file or directory named %s\n", name);
      return FAILURE;
    }
  }

  // 没有同名文件或文件夹，新建一个 inode 和对应的 data block
  Ext2Location inode_location = getFreeInode(file_system->disk);
  unsigned int inode_idx =
      (inode_location.block_idx - INODE_TABLE_BASE) * INODES_PER_BLOCK +
      inode_location.offset / INODE_SIZE;

  // 写入 Dir Entry
  strcpy(entry->name, name);
  entry->name_len = strlen(name);
  entry->file_type = EXT2_FILE;
  entry->inode = inode_idx;
  entry->rec_len = 0;
  addDirEntry(file_system->disk, current, entry);

  // 写入 inode
  Ext2Inode new_inode;
  memset(&new_inode, 0, INODE_SIZE);
  new_inode.mode = 2;
  new_inode.blocks = 1;
  new_inode.size = 0;  // 文件暂无内容
  memset(new_inode.block, 0, sizeof(new_inode.block));
  addInode(file_system->disk, &new_inode, &inode_location);

  return SUCCESS;
}

int ext2Open(Ext2FileSystem *file_system, Ext2Inode *current, char *name) {
  Ext2DirEntry entry;
  BYTE block[BLOCK_SIZE];
  unsigned int items = current->size / DIR_SIZE;
  for (int i = 0; i < items; i++) {
    Ext2Location dir_location =
        getDirEntry(file_system->disk, i, current->block);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(&entry, block + dir_location.offset, DIR_SIZE);
    if (!strcmp(entry.name, name)) {
      if (entry.file_type == EXT2_FILE) {
        printf("It's not a directory!\n");
        return FAILURE;
      }
      memset(block, 0, BLOCK_SIZE);
      unsigned int inode_block = entry.inode / INODES_PER_BLOCK;
      unsigned int inode_offset = entry.inode % INODES_PER_BLOCK;
      readBlock(file_system->disk, INODE_TABLE_BASE + inode_block, block);
      memcpy(current, block + inode_offset * INODE_SIZE, INODE_SIZE);
      return SUCCESS;
    }
  }
  printf("There's no directory named \"%s\"\n", name);
  return FAILURE;
}

void getInodeBitmap(Disk *disk, BYTE bitmap[BLOCK_SIZE]) {
  memset(bitmap, 0, BLOCK_SIZE);
  readBlock(disk, INODE_BITMAP_BASE, bitmap);
}

void getBlockBitmap(Disk *disk, BYTE bitmap[BLOCK_SIZE]) {
  memset(bitmap, 0, BLOCK_SIZE);
  readBlock(disk, BLOCK_BITMAP_BASE, bitmap);
}

void setInodeBitmap(Disk *disk, unsigned int index, int value) {
  BYTE bitmap[BLOCK_SIZE];
  getInodeBitmap(disk, bitmap);
  setBit(bitmap, index, value);
  writeInodeBitmap(disk, bitmap);
}

void setBlockBitmap(Disk *disk, unsigned int index, int value) {
  BYTE bitmap[BLOCK_SIZE];
  getBlockBitmap(disk, bitmap);
  setBit(bitmap, index, value);
  writeBlockBitmap(disk, bitmap);
}

void writeInodeBitmap(Disk *disk, BYTE bitmap[BLOCK_SIZE]) {
  writeBlock(disk, INODE_BITMAP_BASE, bitmap);
}

void writeBlockBitmap(Disk *disk, BYTE bitmap[BLOCK_SIZE]) {
  writeBlock(disk, BLOCK_BITMAP_BASE, bitmap);
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
  if (byte == 0xff) return -1;
  int offset = 0;
  while (1) {
    if ((byte >> (7 - offset)) % 2 == 0) {
      return offset;
    } else {
      offset++;
    }
  }
}

int writeBlock(Disk *disk, unsigned int block_idx, void *block) {
  disk->write_disk(disk, block_idx, block);
  return SUCCESS;
}

int readBlock(Disk *disk, unsigned int block_idx, void *block) {
  disk->read_disk(disk, block_idx, block);
  return SUCCESS;
}