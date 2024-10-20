#include <common.h>
#include <libsio.h>

INCLUDE_ASM("asm/nonmatchings/libsio/libsio", _cdevinput);

INCLUDE_ASM("asm/nonmatchings/libsio/libsio", _cdevscan);

INCLUDE_ASM("asm/nonmatchings/libsio/libsio", _circgetc);

INCLUDE_ASM("asm/nonmatchings/libsio/libsio", _ioabort);

INCLUDE_ASM("asm/nonmatchings/libsio/libsio", add_nullcon_driver);
