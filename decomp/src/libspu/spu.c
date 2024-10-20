#include <common.h>
#include <libspu.h>

INCLUDE_RODATA("asm/nonmatchings/libspu/spu", D_800B4D80);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_init);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FwriteByIO);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FiDMA);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_Fr_);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_t);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_Fw);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_Fr);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FsetRXX);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FsetRXXa);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FgetRXXa);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FsetPCR);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FsetDelayW);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_FsetDelayR);

INCLUDE_ASM("asm/nonmatchings/libspu/spu", _spu_Fw1ts);
