#include "ext2.h"

int writeBlock(Disk* disk, int idx, BYTE* block) {
  for (int i = 0; i < SECTORS_PER_BLOCK; i++) {
    disk->write_sector(disk, idx * SECTORS_PER_BLOCK + i,
                       block + i * BYTES_PER_SECTOR);
  }
}

int readBlock(Disk* disk, int idx, BYTE* block) {
  for (int i = 0; i < SECTORS_PER_BLOCK; i++) {
    disk->read_sector(disk, idx * SECTORS_PER_BLOCK + i,
                      block + i * BYTES_PER_SECTOR);
  }
}