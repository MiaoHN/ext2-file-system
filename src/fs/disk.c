#include "disk.h"

int diskInit(Disk* disk, int num_of_sectors, int byte_per_sector, char* path) {
  if (disk == NULL) return -1;

  disk->path = path;
  disk->number_of_sectors = num_of_sectors;
  disk->bytes_per_sector = byte_per_sector;
  disk->read_sector = diskRead;
  disk->write_sector = diskWrite;

  // 创建指定大小的文件

  FILE* fb = fopen(disk->path, "wb");
  if (fb == NULL) {
    return -1;
  }

  int size = disk->number_of_sectors * disk->bytes_per_sector;
  fseek(fb, 0, SEEK_SET);
  for (int i = 0; i < size; i++) {
    fputc(0, fb);
  }

  fclose(fb);

  return 0;
}

int diskRemove(Disk* disk) {
  if (disk) {
    if (disk->path) {
      remove(disk->path);
    }
  }
}

int diskWrite(Disk* disk, int sector, BYTE* data) {
  if (disk == NULL) return -1;

  // 防止写入数据位置超出文件大小
  if (sector > disk->number_of_sectors) {
    printf("sector is too large!!!\n");
    return -1;
  }

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

int diskRead(Disk* disk, int sector, BYTE* data) {
  if (disk == NULL) return -1;

  // 防止读取数据位置超出文件大小
  if (sector > disk->number_of_sectors) {
    printf("sector is too large!!!\n");
    return -1;
  }

  FILE* fb = fopen(disk->path, "rb");
  if (fb == NULL) {
    printf("Failed to open disk.\n");
    return -1;
  }
  fseek(fb, sector * disk->bytes_per_sector, SEEK_SET);
  fread(data, disk->bytes_per_sector, 1, fb);
  fclose(fb);

  return 0;
}