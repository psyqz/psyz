#include <common.h>
#include <libspu.h>

INCLUDE_ASM("asm/nonmatchings/libspu/t_c", TimerCallback);

INCLUDE_ASM("asm/nonmatchings/libspu/t_c", _tc_cb_trapHSync);

INCLUDE_ASM("asm/nonmatchings/libspu/t_c", _tc_cb_trapVSync);

INCLUDE_ASM("asm/nonmatchings/libspu/t_c", _tc_cb_trapSystemClock);
