#include <common.h>
#include <libgte.h>
#include <libgpu.h>

INCLUDE_ASM("asm/nonmatchings/libgpu/kprintf", PutKanji);

INCLUDE_ASM("asm/nonmatchings/libgpu/kprintf", KanjiFntLoad);

INCLUDE_ASM("asm/nonmatchings/libgpu/kprintf", KanjiFntOpen);

INCLUDE_ASM("asm/nonmatchings/libgpu/kprintf", KanjiFntClose);

INCLUDE_ASM("asm/nonmatchings/libgpu/kprintf", KanjiFntFlush);

INCLUDE_ASM("asm/nonmatchings/libgpu/kprintf", KanjiFntPrint);
