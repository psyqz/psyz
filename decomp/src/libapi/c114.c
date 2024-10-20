#include <common.h>
#include <libapi.h>

__asm__(".pushsection .text\n"
        ".word 0x15007350\n"
        ".word 0x0040809C\n"
        ".popsection");

INCLUDE_ASM("asm/nonmatchings/libapi/c114", _96_remove);
