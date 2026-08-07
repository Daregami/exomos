#include <stdint.h>

#define PAGE_SIZE       4096
#define TOTAL_MEMORY    0x8000000   // 128 МБ
#define TOTAL_PAGES     (TOTAL_MEMORY / PAGE_SIZE)
#define BITMAP_SIZE     (TOTAL_PAGES / 32)