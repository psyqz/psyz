#include <common.h>
#include <libgte.h>

__asm__(".pushsection .text\n"
        ".word 0x15097350\n"
        ".word 0x00409F9C\n"
        ".popsection");

INCLUDE_ASM("asm/nonmatchings/libgte/msc00", InitGeom);
