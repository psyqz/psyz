#include <common.h>
#include <libetc.h>

INCLUDE_ASM("asm/nonmatchings/libetc/intr_dma", startIntrDMA);

WEAK_INCLUDE_ASM("asm/nonmatchings/libetc/intr_dma", trapIntrDMA);

WEAK_INCLUDE_ASM("asm/nonmatchings/libetc/intr_dma", setIntrDMA);

WEAK_INCLUDE_ASM("asm/nonmatchings/libetc/intr_dma", memclr);
