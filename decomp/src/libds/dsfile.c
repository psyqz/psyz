#include <common.h>
#include <libds.h>

INCLUDE_ASM("asm/nonmatchings/libds/dsfile", DsSearchFile);

WEAK_INCLUDE_ASM("asm/nonmatchings/libds/dsfile", _cmp);

WEAK_INCLUDE_ASM("asm/nonmatchings/libds/dsfile", DS_newmedia);

WEAK_INCLUDE_ASM("asm/nonmatchings/libds/dsfile", DS_searchdir);

WEAK_INCLUDE_ASM("asm/nonmatchings/libds/dsfile", DS_cachefile);

WEAK_INCLUDE_ASM("asm/nonmatchings/libds/dsfile", ds_read);
