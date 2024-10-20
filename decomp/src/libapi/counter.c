#include <common.h>
#include <libapi.h>

INCLUDE_ASM("asm/nonmatchings/libapi/counter", SetRCnt);

INCLUDE_ASM("asm/nonmatchings/libapi/counter", GetRCnt);

INCLUDE_ASM("asm/nonmatchings/libapi/counter", StartRCnt);

INCLUDE_ASM("asm/nonmatchings/libapi/counter", StopRCnt);

INCLUDE_ASM("asm/nonmatchings/libapi/counter", ResetRCnt);
