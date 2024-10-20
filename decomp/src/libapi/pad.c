#include <common.h>
#include <libapi.h>

INCLUDE_ASM("asm/nonmatchings/libapi/pad", SetInitPadFlag);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", ReadInitPadFlag);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", PAD_init);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", InitPAD);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", StartPAD);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", StopPAD);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", SetPatchPad);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", RemovePatchPad);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", _Pad1);

INCLUDE_ASM("asm/nonmatchings/libapi/pad", _IsVSync);
