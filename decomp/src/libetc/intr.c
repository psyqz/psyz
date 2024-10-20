#include <common.h>
#include <libetc.h>

INCLUDE_ASM("asm/nonmatchings/libetc/intr", ResetCallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", InterruptCallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", DMACallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", VSyncCallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", VSyncCallbacks);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", StopCallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", RestartCallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", CheckCallback);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", GetIntrMask);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", SetIntrMask);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", startIntr);

const char D_800B2D70[] = "$Id: intr.c,v 1.76 1997/02/12 12:45:05 makoto Exp $";

WEAK_INCLUDE_ASM("asm/nonmatchings/libetc/intr", trapIntr);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", setIntr);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", stopIntr);

INCLUDE_ASM("asm/nonmatchings/libetc/intr", restartIntr);

WEAK_INCLUDE_ASM("asm/nonmatchings/libetc/intr", memclr);
