#include <common.h>
#include <libcd.h>

INCLUDE_ASM("asm/nonmatchings/libcd/event", CdInit);

INCLUDE_ASM("asm/nonmatchings/libcd/event", def_cbsync);

INCLUDE_ASM("asm/nonmatchings/libcd/event", def_cbready);

INCLUDE_ASM("asm/nonmatchings/libcd/event", def_cbread);
