#include <common.h>
#include <libmath.h>

INCLUDE_ASM("asm/nonmatchings/libmath/printf2", printf2);

INCLUDE_ASM("asm/nonmatchings/libmath/printf2", sprintf2);

WEAK_INCLUDE_ASM("asm/nonmatchings/libmath/printf2", prnt);

WEAK_INCLUDE_ASM("asm/nonmatchings/libmath/printf2", cvt);

WEAK_INCLUDE_ASM("asm/nonmatchings/libmath/printf2", round);

WEAK_INCLUDE_ASM("asm/nonmatchings/libmath/printf2", exponent);
