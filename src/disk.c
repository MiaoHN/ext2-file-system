#include "disk.h"

int makeDisk(Disk* disk, const char* path, int number_of_sectors,
             int sector_size, int sectors_per_block) {
  if (disk == NULL) {
    disk = malloc(sizeof(Disk));
  }

  strcpy(disk->path, path);
  disk->disk_info.number_of_sectors = number_of_sectors;
  disk->disk_info.sector_size = sector_size;
  disk->disk_info.sectors_pre_block = sectors_per_block;
  disk->disk_info.disk_size = number_of_sectors * sector_size;
  disk->disk_info.block_size = sector_size * sectors_per_block;
  disk->disk_info.number_of_blocks = number_of_sectors / sectors_per_block;

  disk->write_disk = &writeDisk;
  disk->read_disk = &readDisk;

  FILE* f = fopen(path, "w+");
  // 将磁盘初始信息保存到磁盘开头
  unsigned char block[disk->disk_info.block_size];
  memset(block, 0, disk->disk_info.block_size);
  memcpy(block, &disk->disk_info, sizeof(DiskInfo));
  fseek(f, 0, SEEK_SET);
  fwrite(block, disk->disk_info.block_size, 1, f);

  memset(block, 0, disk->disk_info.block_size);
  for (unsigned int i = 1; i < disk->disk_info.number_of_blocks; i++) {
    fseek(f, i * disk->disk_info.block_size, SEEK_SET);
    fwrite(block, disk->disk_info.block_size, 1, f);
    writeDisk(disk, i, block);
  }
  fclose(f);

  return SUCCESS;
}

int loadDisk(Disk* disk, const char* path) {
  if (disk == NULL) {
    disk = (Disk*)malloc(sizeof(Disk));
  }

  strcpy(disk->path, path);
  disk->read_disk = &readDisk;
  disk->write_disk = &writeDisk;

  DiskInfo disk_info;
  FILE* f = fopen(path, "r+");
  fseek(f, 0, SEEK_SET);
  fread(&disk_info, sizeof(DiskInfo), 1, f);
  fclose(f);

  memcpy(&disk->disk_info, &disk_info, sizeof(DiskInfo));

  return SUCCESS;
}

int writeDisk(Disk* disk, int block_idx, void* data) {
  assert(data != NULL);
  if (block_idx >= disk->disk_info.number_of_sectors) {
    printf("failed to write\n");
    return FAILURE;
  }

  FILE* f = fopen(disk->path, "r+");

  fseek(f, block_idx * disk->disk_info.block_size, SEEK_SET);
  fwrite(data, disk->disk_info.sector_size * disk->disk_info.sectors_pre_block,
         1, f);

  fclose(f);
  return SUCCESS;
}

int readDisk(Disk* disk, int block_idx, void* data) {
  assert(data != NULL);
  if (block_idx >= disk->disk_info.number_of_sectors) {
    printf("failed to write\n");
    return FAILURE;
  }

  memset(data, 0, disk->disk_info.block_size);

  FILE* f = fopen(disk->path, "r+");

  fseek(f, block_idx * disk->disk_info.block_size, SEEK_SET);
  fread(data, disk->disk_info.block_size, 1, f);

  fclose(f);
  return SUCCESS;
}
