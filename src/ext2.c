#include "ext2.h"

int format(Disk* disk) {
  assert(disk != NULL);
  Ext2SuperBlock super_block;
  Ext2GroupDescTable gdt;

  // 初始化超级块和组描述符
  initSuperBlock(&super_block);
  initGdt(&gdt, &super_block);
  // 将超级块和组描述符，两个位图都写入
  writeSuperBlock(disk, &super_block);
  writeGdt(disk, &super_block, &gdt);
  initInodeBitmap(disk, &super_block);
  initBlockBitmap(disk, &super_block);

  initRootDir(disk, &super_block);

  return SUCCESS;
}

int initSuperBlock(Ext2SuperBlock* super_block) {
  super_block->block_group = 0;
  super_block->blocks_count = NUMBER_OF_BLOCKS;
  super_block->blocks_per_group = NUMBER_OF_BLOCKS;
  super_block->inode_size = INODE_SIZE;
  super_block->first_data_block = SUPER_BLOCK_BASE;
  super_block->first_data_block_each_group = DATA_BLOCK_BASE;
  super_block->free_blocks_count = NUMBER_OF_BLOCKS - 1;
  super_block->free_inodes_count = NUMBER_OF_INODES - 11;
  super_block->magic = LINUX;
  super_block->first_ino = 11;
  super_block->errors = 0;
  super_block->log_block_size = 0;
  return SUCCESS;
}

int initGdt(Ext2GroupDescTable* gdt, Ext2SuperBlock* super_block) {
  Ext2GroupDesc gd;
  memset(gdt, 0, sizeof(Ext2GroupDescTable));
  memset(&gd, 0, sizeof(Ext2GroupDesc));

  gd.block_bitmap = BLOCK_BITMAP_BASE;
  gd.inode_bitmap = INODE_BITMAP_BASE;
  gd.free_blocks_count = super_block->free_blocks_count;
  gd.free_inodes_count = (super_block->free_inodes_count + 10) * 2;
  gd.used_dirs_count = 0;
  memcpy(&gdt->table[0], &gd, sizeof(Ext2GroupDesc));
  return SUCCESS;
}

int initInodeBitmap(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  setBit(block, 0, 1);
  writeBlock(disk, INODE_BITMAP_BASE, block);

  return SUCCESS;
}

int initBlockBitmap(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  setBit(block, 0, 1);
  writeBlock(disk, BLOCK_BITMAP_BASE, block);

  return SUCCESS;
}

int initRootDir(Disk* disk, Ext2SuperBlock* super_block) {
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  // 添加根目录
  Ext2DirEntry* entry;
  entry = (Ext2DirEntry*)block;
  strcpy(entry->name, "..");
  entry->name_len = 2;
  entry->file_type = DIR_TYPE;
  entry->inode = 0;

  entry++;
  strcpy(entry->name, ".");
  entry->name_len = 1;
  entry->file_type = DIR_TYPE;
  entry->inode = 0;

  writeBlock(disk, DATA_BLOCK_BASE, block);

  Ext2SuperBlock* read_sp = (Ext2SuperBlock*)block;
  memset(block, 0, BLOCK_SIZE);
  readBlock(disk, SUPER_BLOCK_BASE, block);
  read_sp->free_blocks_count--;
  writeBlock(disk, SUPER_BLOCK_BASE, block);

  Ext2GroupDesc* gd;
  gd = (Ext2GroupDesc*)block;
  memset(block, 0, BLOCK_SIZE);
  readBlock(disk, GDT_BLOCK_BASE, block);
  gd->free_blocks_count--;
  gd->used_dirs_count = 1;
  writeBlock(disk, GDT_BLOCK_BASE, block);
  readBlock(disk, BLOCK_BITMAP_BASE, block);
  setBit(block, super_block->first_data_block_each_group, 1);
  writeBlock(disk, BLOCK_BITMAP_BASE, block);

  readBlock(disk, INODE_BITMAP_BASE, block);
  setBit(block, 0, 1);
  writeBlock(disk, INODE_BITMAP_BASE, block);

  Ext2Inode inode;
  inode.mode = 0x1FF | 0x4000;
  inode.size = 64;
  inode.blocks = 1;
  inode.block[0] = DATA_BLOCK_BASE;
  memcpy(block, &inode, sizeof(Ext2Inode));
  writeBlock(disk, INODE_TABLE_BASE, block);

  return SUCCESS;
}

int writeSuperBlock(Disk* disk, Ext2SuperBlock* super_block) {
  unsigned int block_idx = SUPER_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  memcpy(block, super_block, sizeof(Ext2SuperBlock));
  writeBlock(disk, block_idx, block);

  return SUCCESS;
}

int writeGdt(Disk* disk, Ext2SuperBlock* super_block, Ext2GroupDescTable* gdt) {
  unsigned int block_idx = GDT_BLOCK_BASE;
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);

  memcpy(block, gdt, sizeof(Ext2GroupDescTable));
  writeBlock(disk, block_idx, block);
  return SUCCESS;
}

int getRootInode(Ext2FileSystem* file_system, Ext2Inode* inode) {
  BYTE block[BLOCK_SIZE];
  assert(file_system->disk != NULL);
  readBlock(file_system->disk, INODE_TABLE_BASE, block);
  memcpy(inode, block, sizeof(Ext2Inode));
  return SUCCESS;
}

int addInode(Ext2FileSystem* file_system, Ext2Inode* inode,
             Ext2Location* location) {
  BYTE block[BLOCK_SIZE];
  readBlock(file_system->disk, location->block_idx, block);
  memcpy(block + location->offset, inode, INODE_SIZE);
  writeBlock(file_system->disk, location->block_idx, block);

  return SUCCESS;
}

Ext2Location findFreeInode(Ext2FileSystem* file_system, unsigned int* idx) {
  Ext2Location location;
  BYTE block[BLOCK_SIZE];
  readBlock(file_system->disk, INODE_BITMAP_BASE, block);
  for (int i = 0; i < NUMBER_OF_INODES / 8; i++) {
    if (block[i] != 0xff) {
      // 找到可用空位
      unsigned int offset = getOffset(block[i]);
      block[i] |= (0x80 >> offset);
      // 修改文件系统信息
      writeBlock(file_system->disk, INODE_BITMAP_BASE, block);
      memset(block, 0, BLOCK_SIZE);
      file_system->super_block->free_inodes_count--;
      memcpy(block, file_system->super_block, sizeof(Ext2SuperBlock));
      writeBlock(file_system->disk, SUPER_BLOCK_BASE, block);
      file_system->gdt->table[0].free_inodes_count--;
      memset(block, 0, BLOCK_SIZE);
      memcpy(block, file_system->gdt, sizeof(Ext2GroupDescTable));
      writeBlock(file_system->disk, GDT_BLOCK_BASE, block);
      unsigned int loc = i * 8 + offset;
      *idx = loc;
      location.block_idx = INODE_TABLE_BASE + (loc * INODE_SIZE) / BLOCK_SIZE;
      location.offset = (loc * INODE_SIZE) % BLOCK_SIZE;
      return location;
    }
  }
  location.block_idx = -1;
  location.offset = -1;
  return location;
}

Ext2Location findFreeBlock(Ext2FileSystem* file_system) {
  Ext2Location location;
  BYTE block[BLOCK_SIZE];
  readBlock(file_system->disk, BLOCK_BITMAP_BASE, block);
  for (int i = 0; i < NUMBER_OF_BLOCKS / 8; i++) {
    if (block[i] != 0xff) {
      // 找到可用空位
      unsigned int offset = getOffset(block[i]);
      block[i] |= (0x80 >> offset);
      // 修改文件系统信息
      writeBlock(file_system->disk, BLOCK_BITMAP_BASE, block);
      memset(block, 0, BLOCK_SIZE);
      file_system->super_block->free_blocks_count--;
      memcpy(block, file_system->super_block, sizeof(Ext2SuperBlock));
      writeBlock(file_system->disk, SUPER_BLOCK_BASE, block);
      file_system->gdt->table[0].free_blocks_count--;
      memset(block, 0, BLOCK_SIZE);
      memcpy(block, file_system->gdt, sizeof(Ext2GroupDescTable));
      writeBlock(file_system->disk, GDT_BLOCK_BASE, block);
      unsigned int loc = i * 8 + offset;
      location.block_idx = DATA_BLOCK_BASE + loc;
      location.offset = 0;
      return location;
    }
  }
  location.block_idx = -1;
  location.offset = -1;
  return location;
}

Ext2Location findDirEntry(Ext2FileSystem* file_system, unsigned int index,
                          unsigned int block[8]) {
  Ext2Location location;
  BYTE tmp_block[BLOCK_SIZE];
  unsigned int num;
  unsigned int dir_block = index / (BLOCK_SIZE / DIR_SIZE);
  unsigned int dir_offset = index % (BLOCK_SIZE / DIR_SIZE);
  location.offset = dir_offset * DIR_SIZE;
  if (dir_block < 6) {
    // 直接寻址
    location.block_idx = block[dir_block];
    return location;
  } else {
    dir_block = dir_block - 6;
    if (dir_block < BLOCK_SIZE / 4) {
      // 一个 block 512 字节，一个 int 4 个字节，所以一个 block 可以存储 128
      // 个 block
      readBlock(file_system->disk, block[6], tmp_block);
      memcpy(&num, tmp_block + dir_block * sizeof(unsigned int),
             sizeof(unsigned int));
      location.block_idx = num;
      return location;
    } else {
      // 二级索引
      dir_block = dir_block - BLOCK_SIZE / 4;
      unsigned int block_offset = dir_block / (BLOCK_SIZE / DIR_SIZE);
      readBlock(file_system->disk, block[7], tmp_block);
      memcpy(&num, tmp_block + block_offset * sizeof(unsigned int),
             sizeof(unsigned int));
      readBlock(file_system->disk, num, tmp_block);
      memcpy(&num, tmp_block + dir_block % (BLOCK_SIZE / DIR_SIZE),
             sizeof(unsigned int));
      location.block_idx = num;
      return location;
    }
  }
  location.block_idx = -1;
  return location;
}

int copyInode(Ext2FileSystem* file_system, unsigned int inode_idx,
              Ext2Inode* inode) {
  BYTE block[BLOCK_SIZE];
  unsigned int inode_block = inode_idx / INODES_PER_BLOCK;
  unsigned int inode_offset = inode_idx % INODES_PER_BLOCK;
  readBlock(file_system->disk, INODE_TABLE_BASE + inode_block, block);
  memcpy(inode, block + inode_offset * INODE_SIZE, sizeof(Ext2Inode));
  return SUCCESS;
}

unsigned int addDirEntry(Ext2FileSystem* file_system, Ext2Inode* inode,
                         unsigned int inode_idx, Ext2Location inode_location,
                         int type, char* name) {
  Ext2DirEntry entry;
  entry.file_type = type;
  entry.inode = inode_idx;
  strcpy(entry.name, name);
  BYTE block[BLOCK_SIZE];
  entry.name_len = strlen(name);
  entry.rec_len = 2;
  unsigned int total = inode->size / DIR_SIZE;
  unsigned int dir_block = total / (BLOCK_SIZE / DIR_SIZE);
  unsigned int dir_offset = total % (BLOCK_SIZE / DIR_SIZE);
  if (total < 6) {
    // 直接索引
    readBlock(file_system->disk, inode->block[dir_block], block);
    memcpy(block + dir_offset * DIR_SIZE, &entry, sizeof(Ext2DirEntry));
    writeBlock(file_system->disk, inode->block[dir_block], block);
    inode->size += DIR_SIZE;
    return SUCCESS;
  } else {
    dir_block = dir_block - 6;
    if (inode->blocks < 7) {
      Ext2Location location = findFreeBlock(file_system);
      inode->blocks++;
      inode->block[6] = location.block_idx;
    }
    if (dir_block < BLOCK_SIZE / sizeof(unsigned int)) {
      // 一级索引
      readBlock(file_system->disk, inode->block[6], block);
      memcpy(block + dir_offset * DIR_SIZE, &entry, sizeof(Ext2DirEntry));
      writeBlock(file_system->disk, inode->block[dir_block],
                 block);
      inode->size += DIR_SIZE;
      return SUCCESS;
    } else {
      // 二级索引
      dir_block = dir_block - BLOCK_SIZE / sizeof(unsigned int);
      unsigned block_offset = dir_block / (BLOCK_SIZE / DIR_SIZE);
      unsigned int num;
      readBlock(file_system->disk, inode->block[7], block);
      memcpy(&num, block + block_offset * sizeof(unsigned int),
             sizeof(unsigned int));
      readBlock(file_system->disk, DATA_BLOCK_BASE + num, block);
      memcpy(block + dir_offset * DIR_SIZE, &entry, sizeof(Ext2DirEntry));
      inode->size += DIR_SIZE;
      return SUCCESS;
    }
  }
  return FAILURE;
}

int ext2Ls(Ext2FileSystem* file_system, Ext2Inode* current) {
  unsigned int items = current->size / DIR_SIZE;
  Ext2Inode node;
  Ext2DirEntry dir;
  // 读取 current 对应第一块 block
  BYTE block[BLOCK_SIZE];
  readBlock(file_system->disk, current->block[0], block);
  printf("Type\t\tName\tInode\n");
  for (unsigned int i = 0; i < items; i++) {
    memcpy(&dir, block + sizeof(Ext2DirEntry) * i, sizeof(Ext2DirEntry));
    if (dir.file_type == EXT2_DIR) {
      printf("Dir\t\t");
    } else {
      printf("File\t\t");
    }
    printf("%s\t%d\n", dir.name, dir.inode);
  }
  return SUCCESS;
}

int ext2Mount(Ext2FileSystem* file_system, Ext2Inode* current, char* path) {
  if (file_system->disk == NULL) {
    file_system->disk = (Disk*)malloc(sizeof(Disk));
  }
  if (file_system->super_block == NULL) {
    file_system->super_block = (Ext2SuperBlock*)malloc(sizeof(Ext2SuperBlock));
  }
  if (file_system->gdt == NULL) {
    file_system->gdt = (Ext2GroupDescTable*)malloc(sizeof(Ext2GroupDescTable));
  }
  // 挂载磁盘
  loadDisk(file_system->disk, path);
  // 得到根路径
  getRootInode(file_system, current);
  // 获取 superblock 和 gdt
  BYTE block[BLOCK_SIZE];
  readBlock(file_system->disk, SUPER_BLOCK_BASE, block);
  memcpy(file_system->super_block, block, sizeof(Ext2SuperBlock));
  readBlock(file_system->disk, GDT_BLOCK_BASE, block);
  memcpy(file_system->gdt, block, sizeof(Ext2GroupDescTable));
  return SUCCESS;
}

int ext2Mkdir(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  Ext2DirEntry* entry = (Ext2DirEntry*)malloc(sizeof(Ext2DirEntry));
  BYTE block[BLOCK_SIZE];
  // 查询是否已经存在同名文件
  for (int i = 0; i < current->size / DIR_SIZE; i++) {
    Ext2Location dir_location = findDirEntry(file_system, i, current->block);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(entry, block + dir_location.offset, sizeof(Ext2DirEntry));
    if (!strcmp(entry->name, name)) {
      // 存在同名文件
      printf("There are already a file or directory named %s\n", name);
      return FAILURE;
    }
  }

  // 没有同名文件或文件夹，新建一个 inode 和对应的 data block
  unsigned int inode_idx;
  Ext2Location inode_location = findFreeInode(file_system, &inode_idx);
  Ext2Location block_location = findFreeBlock(file_system);

  // 获得当前目录的 inode 值
  entry = (Ext2DirEntry*)block;
  readBlock(file_system->disk, current->block[0], block);
  while (strcmp(entry->name, ".")) {
    entry++;
  }
  unsigned int parent_inode = entry->inode;
  addDirEntry(file_system, current, inode_idx, inode_location, EXT2_DIR, name);

  // 写入 inode
  Ext2Inode new_inode;
  memset(&new_inode, 0, sizeof(Ext2Inode));
  new_inode.mode = 2;
  new_inode.blocks = 1;           // 当前和上一层目录
  new_inode.size = DIR_SIZE * 2;  // 两个文件夹
  new_inode.block[0] = block_location.block_idx;
  addInode(file_system, &new_inode, &inode_location);

  // 写入 dir entry
  memset(block, 0, BLOCK_SIZE);
  // 写入上级目录
  entry->inode = parent_inode;
  entry->rec_len = sizeof(Ext2DirEntry);
  entry->file_type = EXT2_DIR;
  entry->name_len = 2;
  strcpy(entry->name, "..");
  entry->file_type = EXT2_DIR;
  // 写入当前目录
  entry++;
  entry->inode = inode_idx;
  entry->rec_len = sizeof(Ext2DirEntry);
  entry->file_type = EXT2_DIR;
  entry->name_len = 1;
  strcpy(entry->name, ".");
  entry->file_type = EXT2_DIR;
  // 写入磁盘
  writeBlock(file_system->disk, block_location.block_idx, block);

  return SUCCESS;
}

int ext2Touch(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  Ext2DirEntry* entry = (Ext2DirEntry*)malloc(sizeof(Ext2DirEntry));
  BYTE block[BLOCK_SIZE];
  // 查询是否已经存在同名文件
  for (int i = 0; i < current->size / DIR_SIZE; i++) {
    Ext2Location dir_location = findDirEntry(file_system, i, current->block);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(entry, block + dir_location.offset, sizeof(Ext2DirEntry));
    if (!strcmp(entry->name, name)) {
      // 存在同名文件
      printf("There are already a file or directory named %s\n", name);
      return FAILURE;
    }
  }

  // 没有同名文件或文件夹，新建一个 inode 和对应的 data block
  unsigned int inode_idx;
  Ext2Location inode_location = findFreeInode(file_system, &inode_idx);

  // 获得当前目录的 inode 值
  entry = (Ext2DirEntry*)block;
  readBlock(file_system->disk, current->block[0], block);
  while (strcmp(entry->name, ".")) {
    entry++;
  }
  unsigned int parent_inode = entry->inode;
  addDirEntry(file_system, current, inode_idx, inode_location, EXT2_FILE, name);

  // 写入 inode
  Ext2Inode new_inode;
  memset(&new_inode, 0, sizeof(Ext2Inode));
  new_inode.mode = 2;
  new_inode.blocks = 1;
  new_inode.size = 0;  // 文件暂无内容
  memset(new_inode.block, 0, sizeof(new_inode.block));
  addInode(file_system, &new_inode, &inode_location);

  return SUCCESS;
}

int ext2Open(Ext2FileSystem* file_system, Ext2Inode* current, char* name) {
  Ext2DirEntry entry;
  BYTE block[BLOCK_SIZE];
  for (int i = 0; i < current->size / DIR_SIZE; i++) {
    Ext2Location dir_location = findDirEntry(file_system, i, current->block);
    readBlock(file_system->disk, dir_location.block_idx, block);
    memcpy(&entry, block + dir_location.offset, sizeof(Ext2DirEntry));
    if (!strcmp(entry.name, name)) {
      if (entry.file_type == EXT2_FILE) {
        printf("It's not a directory!\n");
        return FAILURE;
      }
      copyInode(file_system, entry.inode, current);
      return SUCCESS;
    }
  }
  printf("There's no directory named \"%s\"\n", name);
  return FAILURE;
}

int ext2Close(Ext2FileSystem* file_system, Ext2Inode* current) {
  return ext2Open(file_system, current, "..");
}

int setBit(BYTE* block, int index, int value) {
  int byte = index / 8;
  int offset = index % 8;

  if (value == 0)
    block[byte] ^= (0x1 << (7 - index));
  else
    block[byte] |= (0x1 << (7 - index));
  return SUCCESS;
}

int getOffset(BYTE byte) {
  int offset = 0;
  while (1) {
    if ((byte >> (7 - offset)) % 2 == 0) {
      return offset;
    } else {
      offset++;
    }
  }
  return -1;
}

int writeBlock(Disk* disk, unsigned int block_idx, void* block) {
  disk->write_disk(disk, block_idx, block);
  return SUCCESS;
}

int readBlock(Disk* disk, unsigned int block_idx, void* block) {
  disk->read_disk(disk, block_idx, block);
  return SUCCESS;
}