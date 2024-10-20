#include <common.h>
#include <libds.h>

INCLUDE_ASM("asm/nonmatchings/libds/dsplay", DsPlay);

WEAK_INCLUDE_ASM("asm/nonmatchings/libds/dsplay", play);

INCLUDE_ASM("asm/nonmatchings/libds/dsplay", stop);

INCLUDE_ASM("asm/nonmatchings/libds/dsplay", cbready);

INCLUDE_RODATA("asm/nonmatchings/libds/dsplay", D_800B2B24);

INCLUDE_RODATA("asm/nonmatchings/libds/dsplay", D_800B2B3C);
