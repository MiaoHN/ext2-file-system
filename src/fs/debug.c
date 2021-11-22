#include "debug.h"

int printDump(BYTE* data, int offset, int bytes) {
  for (int i = 0; i < bytes - 2; i = i + 2) {
    BYTE data1 = data[i];
    BYTE data2 = data[i + 1];
    BYTE high1 = (data1 & 0xf0) >> 4;
    BYTE low1 = (data1 & 0x0f) >> 0;
    BYTE high2 = (data2 & 0xf0) >> 4;
    BYTE low2 = (data2 & 0x0f) >> 0;
    printf("%x%x%x%x ", high2, low2, high1, low1);
  }
}

int dumpDisk(Disk* disk, int offset, int bytes) {
  FILE* fb = fopen(disk->path, "rb");
  if (fb == NULL) return -1;

  if ((offset + bytes) > (disk->number_of_sectors * disk->bytes_per_sector)) {
    printf("out of range\n");
  }

  BYTE* data;
  memset(data, 0, bytes);
  fseek(fb, offset, SEEK_SET);
  fread(data, bytes * sizeof(BYTE), 1, fb);

  printDump(data, offset, bytes);

  return 0;
}