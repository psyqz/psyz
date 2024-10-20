#include <common.h>
#include <libgte.h>
#include <libgpu.h>

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", OpenTIM);

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", ReadTIM);

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", OpenTMD);

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", ReadTMD);

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", get_tim_addr);

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", get_tmd_addr);

INCLUDE_ASM("asm/nonmatchings/libgpu/tmd", unpack_packet);
