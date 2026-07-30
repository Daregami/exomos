#include "vga.h"

static volatile char *video_buf = (volatile char*)VGA_ADDR;
static int cursor[] = {0,0};

void kernel_log(char msg, int type) {
    switch (type) {
        case VGA_OK:
            video_buf[80*3*2] = 'Y';
            video_buf[80*3*2 + 1] = COLOR_WHITE;
        case VGA_ERROR:
            video_buf[80*3*2] = 'N';
            video_buf[80*3*2 + 1] = COLOR_WHITE;
    }
}
