#include <common.h>
#include <libc.h>

INCLUDE_ASM("asm/nonmatchings/libc2/malloc", malloc);

INCLUDE_ASM("asm/nonmatchings/libc2/malloc", free);

INCLUDE_ASM("asm/nonmatchings/libc2/malloc", calloc);

INCLUDE_ASM("asm/nonmatchings/libc2/malloc", realloc);
