#include <common.h>
#include <libcd.h>

const char D_800B2320[] = "CdlReadS";
const char D_800B232C[] = "CdlSeekP";
const char D_800B2338[] = "CdlSeekL";
const char D_800B2344[] = "CdlGetTD";
const char D_800B2350[] = "CdlGetTN";
const char D_800B235C[] = "CdlGetlocP";
const char D_800B2368[] = "CdlGetlocL";
const char D_800B2374[] = "?";
const char D_800B2378[] = "CdlSetmode";
const char D_800B2384[] = "CdlSetfilter";
const char D_800B2394[] = "CdlDemute";
const char D_800B23A0[] = "CdlMute";
const char D_800B23A8[] = "CdlReset";
const char D_800B23B4[] = "CdlPause";
const char D_800B23C0[] = "CdlStop";
const char D_800B23C8[] = "CdlStandby";
const char D_800B23D4[] = "CdlReadN";
const char D_800B23E0[] = "CdlBackward";
const char D_800B23EC[] = "CdlForward";
const char D_800B23F8[] = "CdlPlay";
const char D_800B2400[] = "CdlSetloc";
const char D_800B240C[] = "CdlNop";
const char D_800B2414[] = "CdlSync";
const char D_800B241C[] = "DiskError";
const char D_800B2428[] = "DataEnd";
const char D_800B2430[] = "Acknowledge";
const char D_800B243C[] = "Complete";
const char D_800B2448[] = "DataReady";
const char D_800B2454[] = "NoIntr";

INCLUDE_RODATA("asm/nonmatchings/libcd/bios", D_800B245C);

INCLUDE_RODATA("asm/nonmatchings/libcd/bios", D_800B246C);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", getintr);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_sync);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_ready);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_cw);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_vol);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_flush);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_initvol);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_initintr);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_init);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_datasync);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_getsector);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_getsector2);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", CD_set_test_parmnum);

INCLUDE_ASM("asm/nonmatchings/libcd/bios", callback);
