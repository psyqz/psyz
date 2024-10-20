#include <common.h>
#include <libcard.h>

INCLUDE_ASM("asm/nonmatchings/libcard/init", InitCARD);

INCLUDE_ASM("asm/nonmatchings/libcard/init", StartCARD);

INCLUDE_ASM("asm/nonmatchings/libcard/init", StopCARD);
