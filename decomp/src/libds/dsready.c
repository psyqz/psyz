#include <common.h>
#include <libds.h>

INCLUDE_ASM("asm/nonmatchings/libds/dsready", DsStartReadySystem);

INCLUDE_ASM("asm/nonmatchings/libds/dsready", DsEndReadySystem);

INCLUDE_ASM("asm/nonmatchings/libds/dsready", ER_cbready);

INCLUDE_ASM("asm/nonmatchings/libds/dsready", ER_retry);

INCLUDE_ASM("asm/nonmatchings/libds/dsready", ER_cbsync);
