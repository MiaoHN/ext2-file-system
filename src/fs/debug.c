#include "debug.h"

int dumpDisk(Disk* disk, int offset, int bytes) {
  FILE* fb = fopen(disk->path, "rb");
  if (fb == NULL) return -1;

  if ((offset + bytes) > (disk->number_of_sectors * disk->bytes_per_sector)) {
    printf("out of range\n");
  }

  int base = offset & 0xffffff00;
  int lines = bytes / 16 + 1;

  BYTE* data = calloc(1, sizeof(BYTE));
  memset(data, 0, lines * 2 * sizeof(BYTE));
  fseek(fb, base, SEEK_SET);
  fread(data, lines * 16 * sizeof(BYTE), 1, fb);

  for (int i = 0; i < lines; i++) {
    printf("%.8x  ", base + 16 * i);
    for (int ii = 0; ii < 16; ii++) {
      BYTE high = (data[i * 16 + ii] & 0xf0) >> 4;
      BYTE low = (data[i * 16 + ii] & 0x0f) >> 0;
      if (ii == 8) {
        printf(" %x%x ", high, low);
      } else {
        printf("%x%x ", high, low);
      }
    }
    printf(" |");

    for (int ii = 0; ii < 16; ii++) {
      unsigned char ch = data[i * 16 + ii];
      if (ch == '\0') ch = '.';
      printf("%c", ch);
    }

    printf("|\n");
  }

  free(data);

  return 0;
}