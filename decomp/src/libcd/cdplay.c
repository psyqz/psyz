#include <common.h>
#include <libcd.h>

INCLUDE_ASM("asm/nonmatchings/libcd/cdplay", CdPlay);

INCLUDE_ASM("asm/nonmatchings/libcd/cdplay", cbdataready);

WEAK_INCLUDE_ASM("asm/nonmatchings/libcd/cdplay", play);
