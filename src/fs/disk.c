#include "disk.h"

/**
 * @brief 更新磁盘文件
 *
 * @param disk 待更新的磁盘
 * @return int 正常返回 0
 */
int updateDisk(Disk* disk) {
  if (disk == NULL) return -1;

  FILE* fb = fopen(disk->path, "wb");
  if (fb == NULL) {
    return -1;
  }

  int size = sizeof(char) * disk->num_of_sector * disk->bytes_per_sector;
  // 写入 num_of_sectors * byte_per_sector * 8 位 0
  fwrite(disk->data, size, 1, fb);

  fclose(fb);
  return 0;
}

int diskInit(Disk* disk, int num_of_sectors, int byte_per_sector,
             DISK_PATH path) {
  if (disk == NULL) return -1;

  disk->path = path;
  disk->num_of_sector = num_of_sectors;
  disk->bytes_per_sector = byte_per_sector;
  disk->read_sector = diskRead;
  disk->write_sector = diskWrite;

  // 创建指定大小的文件

  int size = sizeof(char) * num_of_sectors * byte_per_sector;
  // 写入 num_of_sectors * byte_per_sector * 8 位 0
  disk->data = (DATA)malloc(size);
  for (int i = 0; i < size; i++) {
    disk->data[i] = '\0';
  }

  updateDisk(disk);

  return 0;
}

int diskUnInit(Disk* disk) {
  if (disk) {
    if (disk->path) {
      remove(disk->path);
    }
  }
}

int diskLoad(DISK_PATH path) {
  // TODO
  return 0;
}

int diskWrite(Disk* disk, int sector, DATA data) {
  int position = sector * disk->bytes_per_sector;
  for (int i = 0; i < disk->bytes_per_sector; i++) {
    disk->data[position + i] = data[i];
  }

  updateDisk(disk);

  return 0;
}

int diskRead(Disk* disk, int sector, DATA data) {
  int position = sector * disk->bytes_per_sector;

  for (int i = 0; i < disk->bytes_per_sector; i++) {
    data[i] = disk->data[position + i];
  }

  return 0;
}