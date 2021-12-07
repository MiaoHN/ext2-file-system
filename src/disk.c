#include "disk.h"

int makeDisk(Disk* disk, const char* path) {
  if (disk == NULL) {
    disk = malloc(sizeof(Disk));
  }

  strcpy(disk->path, path);

  disk->write_disk = &writeDisk;
  disk->read_disk = &readDisk;

  FILE* f = fopen(path, "w+");
  BYTE block[BLOCK_SIZE];
  memset(block, 0, BLOCK_SIZE);
  for (unsigned int i = 0; i < NUMBER_OF_BLOCKS; i++) {
    fseek(f, i * BLOCK_SIZE, SEEK_SET);
    fwrite(block, BLOCK_SIZE, 1, f);
    writeDisk(disk, i, block);
  }
  fclose(f);

  printf("Successfully make disk named \"%s\"!\n", path);
  printf("    Total Size:       %d bytes\n", BLOCK_SIZE * NUMBER_OF_BLOCKS);
  printf("    Number of Blocks: %d\n", NUMBER_OF_BLOCKS);
  printf("    Block Size:       %d bytes\n", BLOCK_SIZE);
  printf("    Sector Size:      %d bytes\n", SECTOR_SIZE);
  printf("    Sectors per Blocks: %d\n\n", SECTORS_PRE_BLOCK);
  return SUCCESS;
}

int loadDisk(Disk* disk, const char* path) {
  if (disk == NULL) {
    disk = (Disk*)malloc(sizeof(Disk));
  }

  strcpy(disk->path, path);
  disk->read_disk = &readDisk;
  disk->write_disk = &writeDisk;

  return SUCCESS;
}

int writeDisk(Disk* disk, unsigned int block_idx, void* data) {
  assert(data != NULL);
  if (block_idx >= NUMBER_OF_BLOCKS) {
    printf("failed to write\n");
    return FAILURE;
  }

  FILE* f = fopen(disk->path, "r+");

  fseek(f, block_idx * BLOCK_SIZE, SEEK_SET);
  fwrite(data, BLOCK_SIZE, 1, f);

  fclose(f);
  return SUCCESS;
}

int readDisk(Disk* disk, unsigned int block_idx, void* data) {
  assert(data != NULL);
  if (block_idx >= NUMBER_OF_BLOCKS) {
    printf("failed to read\n");
    return FAILURE;
  }

  memset(data, 0, BLOCK_SIZE);

  FILE* f = fopen(disk->path, "r+");

  fseek(f, block_idx * BLOCK_SIZE, SEEK_SET);
  fread(data, BLOCK_SIZE, 1, f);

  fclose(f);
  return SUCCESS;
}
