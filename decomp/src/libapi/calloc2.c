#include <common.h>
#include <libapi.h>

INCLUDE_ASM("asm/nonmatchings/libapi/calloc2", calloc2);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/calloc2", _bzero);
