#ifndef PSYZ_H
#define PSYZ_H

#include <types.h>
#include <log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifdef __psyz
// stub unix implementations
#define open my_open
#define close my_close
#define lseek my_lseek
#define read my_read
#define write my_write
#define ioctl my_ioctl
// int my_open(const char *devname, int flag, ...);
int my_open(const char* devname, int flag);
int my_close(int fd);
long my_lseek(long fd, long offset, long flag);
long my_read(long fd, void* buf, long n);
long my_write(long fd, void* buf, long n);
long my_ioctl(long fd, long com, long arg);
#endif

#define PAD_COUNT 2
#define PAD_L2 0x0001
#define PAD_R2 0x0002
#define PAD_L1 0x0004
#define PAD_R1 0x0008
#define PAD_TRIANGLE 0x0010
#define PAD_CIRCLE 0x0020
#define PAD_CROSS 0x0040
#define PAD_SQUARE 0x0080
#define PAD_SELECT 0x0100
#define PAD_L3 0x0200
#define PAD_R3 0x0400
#define PAD_START 0x0800
#define PAD_UP 0x1000
#define PAD_RIGHT 0x2000
#define PAD_DOWN 0x4000
#define PAD_LEFT 0x8000

#define DISP_WIDTH 256
#define DISP_HEIGHT 256
#define SCREEN_SCALE 2
#define VRAM_W 1024
#define VRAM_H 512
#define VRAM_STRIDE 2048

#define NOP
#define CLAMP(x, min, max) x < min ? min : (x > max ? max : x)

#ifndef LEN
#define LEN(x) ((s32)(sizeof(x) / sizeof(*(x))))
#endif

#ifndef LENU
#define LENU(x) ((u32)(sizeof(x) / sizeof(*(x))))
#endif

#endif
