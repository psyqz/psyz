#include <common.h>
#include <libmcrd.h>

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardInit);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardEnd);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardStart);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardStop);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardExist);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_exist_back);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardAccept);

INCLUDE_RODATA("asm/nonmatchings/libmcrd/libmcrd", D_800B45B8);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_accept_back);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardOpen);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardClose);

INCLUDE_RODATA("asm/nonmatchings/libmcrd/libmcrd", D_800B4688);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardReadData);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_read_data);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardWriteData);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_write_data);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardReadFile);

INCLUDE_RODATA("asm/nonmatchings/libmcrd/libmcrd", D_800B46E4);

INCLUDE_RODATA("asm/nonmatchings/libmcrd/libmcrd", D_800B4708);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_read_file);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardWriteFile);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_write_file);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardGetDirentry);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardCallback);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardSync);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardCreateFile);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardDeleteFile);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardFormat);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", MemCardUnformat);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _rslt_to_code);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _callback_handler);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _make_file_discrpt);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpIOE);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpError);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpTimeout);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpNewcard);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpIOEx);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpErrorx);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpTimeoutx);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", funcEvSpNewcardx);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_open);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_start);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_close);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _card_stop);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _clr_card_event);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _get_card_event);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _get_card_event_x);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _chk_card_event);

INCLUDE_ASM("asm/nonmatchings/libmcrd/libmcrd", _chk_card_event_x);

INCLUDE_RODATA("asm/nonmatchings/libmcrd/libmcrd", D_800B4824);
