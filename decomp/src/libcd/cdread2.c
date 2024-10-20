#include <common.h>
#include <libcd.h>

INCLUDE_ASM("asm/nonmatchings/libcd/cdread2", CdRead2);

WEAK_INCLUDE_ASM("asm/nonmatchings/libcd/cdread2", StCdInterrupt2);
