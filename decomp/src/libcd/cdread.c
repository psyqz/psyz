#include <common.h>
#include <libcd.h>

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", cb_read);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", cb_data);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", cd_read_retry);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", CdReadBreak);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", CdRead);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", CdReadSync);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", CdReadCallback);

INCLUDE_ASM("asm/nonmatchings/libcd/cdread", CdReadMode);
