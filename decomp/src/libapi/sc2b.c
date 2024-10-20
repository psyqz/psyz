#include <common.h>
#include <libapi.h>

INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", SetConf);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", ResetEntryInt);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", InitRCnt);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", InitException);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", SysInitMemory);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", InitDefInt);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", SysMalloc);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", _SysMalloc);

WEAK_INCLUDE_ASM("asm/nonmatchings/libapi/sc2b", _init);
