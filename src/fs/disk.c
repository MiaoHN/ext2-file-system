#include "disk.h"

int diskInit(Disk* disk, int num_of_sectors, int byte_per_sector,
             DISK_PATH path) {
  if (disk == NULL) return -1;

  disk->path = path;
  disk->num_of_sector = num_of_sectors;
  disk->bytes_per_sector = byte_per_sector;
  disk->read_sector = diskRead;
  disk->write_sector = diskWrite;

  // 创建指定大小的文件

  FILE* fb = fopen(disk->path, "wb");
  if (fb == NULL) {
    return -1;
  }

  int size = sizeof(char) * disk->num_of_sector * disk->bytes_per_sector;
  // 写入 num_of_sectors * byte_per_sector * 8 位 0
  fwrite('\0', sizeof(char), size, fb);

  fclose(fb);

  return 0;
}

int diskUnInit(Disk* disk) {
  if (disk) {
    if (disk->path) {
      remove(disk->path);
    }
  }
}

int diskWrite(Disk* disk, int sector, DATA data) {
  if (disk == NULL) return -1;

  FILE* fb = fopen(disk->path, "rb+");
  if (fb == NULL) {
    printf("Failed to open disk.\n");
    return -1;
  }
  fseek(fb, sector * disk->bytes_per_sector, SEEK_SET);
  fwrite(data, disk->bytes_per_sector, 1, fb);

  fclose(fb);
  return 0;
}

int diskRead(Disk* disk, int sector, DATA data) {
  if (disk == NULL) return -1;

  FILE* fb = fopen(disk->path, "rb+");
  if (fb == NULL) {
    printf("Failed to open disk.\n");
    return -1;
  }
  fseek(fb, sector * disk->bytes_per_sector, SEEK_SET);
  fread(data, disk->bytes_per_sector, 1, fb);
  fclose(fb);

  return 0;
}