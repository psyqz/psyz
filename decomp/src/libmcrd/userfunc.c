#include <common.h>
#include <libmcrd.h>

INCLUDE_ASM("asm/nonmatchings/libmcrd/userfunc", UserFuncInit);

INCLUDE_ASM("asm/nonmatchings/libmcrd/userfunc", UserFuncOpen);

INCLUDE_ASM("asm/nonmatchings/libmcrd/userfunc", UserFuncExecute);

INCLUDE_ASM("asm/nonmatchings/libmcrd/userfunc", UserFuncComplete);
