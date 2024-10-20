#include <common.h>
#include <libapi.h>

INCLUDE_ASM("asm/nonmatchings/libapi/calloc3", calloc3);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/calloc3", _bzero);
