#include <common.h>
#include <libgte.h>
#include <libgpu.h>

INCLUDE_ASM("asm/nonmatchings/libgpu/break", BreakDraw);

INCLUDE_ASM("asm/nonmatchings/libgpu/break", IsIdleGPU);

INCLUDE_ASM("asm/nonmatchings/libgpu/break", ContinueDraw);
