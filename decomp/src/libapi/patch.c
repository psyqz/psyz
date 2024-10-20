#include <common.h>
#include <libapi.h>

INCLUDE_ASM("asm/nonmatchings/libapi/patch", EnablePAD);

INCLUDE_ASM("asm/nonmatchings/libapi/patch", DisablePAD);

INCLUDE_ASM("asm/nonmatchings/libapi/patch", _patch_pad);
