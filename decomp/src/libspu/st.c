#include <common.h>
#include <libspu.h>

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStCBPrepare);

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStStartPrepare);

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStSetTransferEnv);

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStCB_IRQ);

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStCB_Transfer);

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStStart);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStTransfer);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStInit);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStQuit);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStGetStatus);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStGetVoiceStatus);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStSetPreparationFinishedCallback);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStSetTransferFinishedCallback);

INCLUDE_ASM("asm/nonmatchings/libspu/st", SpuStSetStreamFinishedCallback);

INCLUDE_ASM("asm/nonmatchings/libspu/st", _SpuStCB_IRQfinal);
